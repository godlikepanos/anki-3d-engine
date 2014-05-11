#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/photoshop_filters.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

layout(binding = 0) uniform lowp sampler2D uIsRt;
layout(binding = 1) uniform lowp sampler2D uPpsSsaoRt;
layout(binding = 2) uniform lowp sampler2D uPpsHdrRt;
layout(binding = 3) uniform lowp sampler2D uPpsLfRt;

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec3 outColor;

const vec2 TEX_OFFSET = vec2(1.0 / float(FBO_WIDTH), 1.0 / float(FBO_HEIGHT));

const vec2 KERNEL[8] = vec2[](
	vec2(TEX_OFFSET.x, TEX_OFFSET.y),
	vec2(0.0, TEX_OFFSET.y),
	vec2(-TEX_OFFSET.x, TEX_OFFSET.y),
	vec2(-TEX_OFFSET.x, 0.0),
	vec2(-TEX_OFFSET.x, -TEX_OFFSET.y),
	vec2(0.0, -TEX_OFFSET.y),
	vec2(TEX_OFFSET.x, -TEX_OFFSET.y),
	vec2(TEX_OFFSET.x, 0.0));

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
vec3 sharpen(in sampler2D tex, in vec2 texCoords)
{
	const float sharpenFactor = 0.25;

	vec3 col = textureRt(tex, texCoords).rgb;

	vec3 col2 = textureRt(tex, texCoords + KERNEL[0]).rgb;
	for(int i = 1; i < 8; i++)
	{
		col2 += textureRt(tex, texCoords + KERNEL[i]).rgb;
	}

	return col * (8.0 * sharpenFactor + 1.0) - sharpenFactor * col2;
}

//==============================================================================
vec3 erosion(in sampler2D tex, in vec2 texCoords)
{
    vec3 minValue = textureRt(tex, texCoords).rgb;

    for (int i = 0; i < 8; i++)
    {
        vec3 tmpCol = textureRt(tex, texCoords + KERNEL[i]).rgb;
        minValue = min(tmpCol, minValue);
    }

    return minValue;
}

//==============================================================================
void main()
{
#if SHARPEN_ENABLED
	outColor = sharpen(uIsRt, inTexCoords);
#else
	outColor = textureRt(uIsRt, inTexCoords).rgb;
#endif
	//outColor = erosion(uIsRt, inTexCoords);

#if HDR_ENABLED
	vec3 hdr = textureRt(uPpsHdrRt, inTexCoords).rgb;
	outColor += hdr;
#endif

#if SSAO_ENABLED
	float ssao = textureRt(uPpsSsaoRt, inTexCoords).r;
	outColor *= ssao;
#endif

#if LF_ENABLED
	vec3 lf = textureRt(uPpsLfRt, inTexCoords).rgb;
	outColor += lf;
#endif

#if GAMMA_CORRECTION_ENABLED
	//outColor = BlendHardLight(vec3(0.7, 0.72, 0.4), outColor);
	outColor = gammaCorrectionRgb(vec3(0.9, 0.92, 0.75), outColor);
#endif

#if 0
	if(outColor.r != 0.00000001)
	{
		outColor = vec3(ssao);
	}
#endif

#if 0
	if(outColor.r != 0.00000001)
	{
		outColor = hdr;
	}
#endif
}

