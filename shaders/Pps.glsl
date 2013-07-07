#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader
#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/photoshop_filters.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

uniform highp sampler2D msDepthFai;
uniform lowp sampler2D isFai;
uniform lowp sampler2D ppsHdrFai;
uniform lowp sampler2D ppsSsaoFai;
uniform lowp sampler2D ppsLfFai;

in vec2 vTexCoords;

layout(location = 0) out vec3 fColor;

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

	vec3 col = texture(tex, texCoords).rgb;

	vec3 col2 = texture(tex, texCoords + KERNEL[0]).rgb;
	for(int i = 1; i < 8; i++)
	{
		col2 += texture(tex, texCoords + KERNEL[i]).rgb;
	}

	return col * (8.0 * sharpenFactor + 1.0) - sharpenFactor * col2;
}

//==============================================================================
vec3 erosion(in sampler2D tex, in vec2 texCoords)
{
    vec3 minValue = texture(tex, texCoords).rgb;

    for (int i = 0; i < 8; i++)
    {
        vec3 tmpCol = textureLod(tex, texCoords + KERNEL[i], 0.0).rgb;
        minValue = min(tmpCol, minValue);
    }

    return minValue;
}

//==============================================================================
void main(void)
{
#if defined(SHARPEN_ENABLED)
	fColor = sharpen(isFai, vTexCoords);
#else
	fColor = texture(isFai, vTexCoords).rgb;
#endif
	//fColor = erosion(isFai, vTexCoords);

#if defined(HDR_ENABLED)
	vec3 hdr = texture(ppsHdrFai, vTexCoords).rgb;
	fColor += hdr;
#endif

#if defined(SSAO_ENABLED)
	float ssao = texture(ppsSsaoFai, vTexCoords).r;
	fColor *= ssao;
#endif

#if defined(LF_ENABLED)
	vec3 lf = texture(ppsLfFai, vTexCoords).rgb;
	fColor += lf;
#endif

	fColor = gammaCorrectionRgb(vec3(0.9, 0.92, 0.75), fColor);

#if 0
	if(fColor.r != 0.00000001)
	{
		fColor = vec3(ssao);
	}
#endif

#if 0
	if(fColor.r != 0.00000001)
	{
		fColor = hdr;
	}
#endif
}

