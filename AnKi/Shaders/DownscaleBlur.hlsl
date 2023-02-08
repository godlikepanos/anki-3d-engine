// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/TonemappingFunctions.hlsl>
#include <AnKi/Shaders/Functions.hlsl>

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec3> g_tex;

constexpr U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.hlsl>

struct Uniforms
{
	UVec2 m_fbSize;
	U32 m_revertTonemapping;
	U32 m_padding;
};
[[vk::push_constant]] ConstantBuffer<Uniforms> g_uniforms;

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#endif

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec4 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(svDispatchThreadId.x >= g_uniforms.m_fbSize.x || svDispatchThreadId.y >= g_uniforms.m_fbSize.y)
	{
		// Skip pixels outside the viewport
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(g_uniforms.m_fbSize);
#endif

	RVec3 output;
	const RF32 weight = 1.0 / 5.0;
	output = g_tex.SampleLevel(g_linearAnyClampSampler, uv, 0.0) * weight;
	output += g_tex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, +1)) * weight;
	output += g_tex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, -1)) * weight;
	output += g_tex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, -1)) * weight;
	output += g_tex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, +1)) * weight;

	if(g_uniforms.m_revertTonemapping != 0u)
	{
		output = saturate(output);
		output = sRgbToLinear(output);
		output = invertTonemap(output, readExposureAndAverageLuminance().x);
	}

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = RVec4(output, 1.0);
#else
	return output;
#endif
}
