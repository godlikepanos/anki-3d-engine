// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/TonemappingFunctions.hlsl>
#include <AnKi/Shaders/Functions.hlsl>
constexpr U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.hlsl>

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_inTex;

struct Constants
{
	F32 m_threshold;
	F32 m_scale;
	F32 m_padding0;
	F32 m_padding1;
};

[[vk::push_constant]] ConstantBuffer<Constants> g_consts;

#if defined(ANKI_COMPUTE_SHADER)
#	define THREADGROUP_SIZE_X 8
#	define THREADGROUP_SIZE_Y 8
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#endif

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y, 1)] void main(UVec2 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	Vec2 outUavSize;
	g_outUav.GetDimensions(outUavSize.x, outUavSize.y);

	const Vec2 uv = (Vec2(svDispatchThreadId) + 0.5) / outUavSize;
#endif

	const RF32 weight = 1.0 / 5.0;
	RVec3 color = g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, +1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, -1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, +1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, -1)).rgb * weight;

	color = tonemap(color, readExposureAndAverageLuminance().y, g_consts.m_threshold) * g_consts.m_scale;

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId] = RVec4(color, 0.0);
#else
	return color;
#endif
}
