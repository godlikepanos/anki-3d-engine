// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Does tonemapping

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/TonemappingFunctions.hlsl>

[[vk::binding(0)]] SamplerState g_nearestAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_inputRt;

constexpr U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.hlsl>

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#	define THREADGROUP_SIZE_SQRT 8
#endif

struct Uniforms
{
	Vec2 m_viewportSizeOverOne;
	UVec2 m_viewportSize;
};

[[vk::push_constant]] ConstantBuffer<Uniforms> g_uniforms;

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(THREADGROUP_SIZE_SQRT, THREADGROUP_SIZE_SQRT, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main(Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(UVec2(THREADGROUP_SIZE_SQRT, THREADGROUP_SIZE_SQRT), g_uniforms.m_viewportSize,
								  svDispatchThreadId.xy))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5f) * g_uniforms.m_viewportSizeOverOne;
#endif

	const RVec3 hdr = g_inputRt.SampleLevel(g_nearestAnyClampSampler, uv, 0.0f).rgb;
	const Vec3 tonemapped = linearToSRgb(tonemap(hdr, readExposureAndAverageLuminance().x));

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = RVec4(tonemapped, 0.0);
#else
	return tonemapped;
#endif
}
