// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Tonemapping.glsl"
#include "shaders/Functions.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_isRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_ppsBloomLfRt;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler3D u_lut;

struct Luminance
{
	vec4 averageLuminancePad3;
};

layout(std140, ANKI_SS_BINDING(0, 0)) readonly buffer s0_
{
	Luminance u_luminance;
};

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec3 out_color;

const vec2 TEX_OFFSET = vec2(1.0 / float(FBO_WIDTH), 1.0 / float(FBO_HEIGHT));

const vec2 KERNEL[8] = vec2[](vec2(TEX_OFFSET.x, TEX_OFFSET.y),
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

	vec3 col = textureLod(tex, texCoords, 0.0).rgb;

	vec3 col2 = textureLod(tex, texCoords + KERNEL[0], 0.0).rgb;
	for(int i = 1; i < 8; i++)
	{
		col2 += textureLod(tex, texCoords + KERNEL[i], 0.0).rgb;
	}

	col = col * (8.0 * sharpenFactor + 1.0) - sharpenFactor * col2;

	return max(col, vec3(EPSILON));
}

//==============================================================================
vec3 erosion(in sampler2D tex, in vec2 texCoords)
{
	vec3 minValue = textureRt(tex, texCoords).rgb;

	for(int i = 0; i < 8; i++)
	{
		vec3 tmpCol = textureRt(tex, texCoords + KERNEL[i]).rgb;
		minValue = min(tmpCol, minValue);
	}

	return minValue;
}

//==============================================================================
vec3 colorGrading(in vec3 color)
{
	const vec3 LUT_SCALE = vec3((LUT_SIZE - 1.0) / LUT_SIZE);
	const vec3 LUT_OFFSET = vec3(1.0 / (2.0 * LUT_SIZE));

	color = min(color, vec3(1.0));
	vec3 lutCoords = color * LUT_SCALE + LUT_OFFSET;
	return textureLod(u_lut, lutCoords, 0.0).rgb;
}

//==============================================================================
void main()
{
#if SHARPEN_ENABLED
	out_color = sharpen(u_isRt, in_uv);
#else
	out_color = textureLod(u_isRt, in_uv, 0.0).rgb;
#endif

	out_color = tonemap(out_color, u_luminance.averageLuminancePad3.x, 0.0);

#if BLOOM_ENABLED
	vec3 bloom = textureLod(u_ppsBloomLfRt, in_uv, 0.0).rgb;
	out_color += bloom;
#endif

	out_color = colorGrading(out_color);

#if 0
	{
		out_color = bloom;
	}
#endif
}
