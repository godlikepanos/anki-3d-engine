// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/TonemappingFunctions.hlsl>
#include <AnKi/Shaders/Functions.hlsl>
constexpr U32 kTonemappingBinding = 2u;
#include <AnKi/Shaders/TonemappingResources.hlsl>

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kViewport, 0u);

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D g_inTex; ///< Its the IS RT

struct Uniforms
{
	Vec4 m_thresholdScalePad2;
};

[[vk::push_constant]] ConstantBuffer<Uniforms> g_pc;

struct CompIn
{
	UVec3 m_svDispatchThreadId : SV_DISPATCHTHREADID;
};

struct VertOut
{
	Vec4 m_position : SV_POSITION;
	[[vk::location(0)]] Vec2 m_uv : TEXCOORD;
};

struct FragOut
{
	RVec3 m_svTarget : SV_TARGET0;
};

#if defined(ANKI_COMPUTE_SHADER)
#	define THREADGROUP_SIZE_X 8
#	define THREADGROUP_SIZE_Y 8
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#endif

#if defined(ANKI_COMPUTE_SHADER)
ANKI_NUMTHREADS(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y, 1) void main(CompIn input)
#else
FragOut main(VertOut input)
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(UVec2(THREADGROUP_SIZE_X, THREADGROUP_SIZE_Y), kViewport, input.m_svDispatchThreadId))
	{
		return;
	}

	const Vec2 uv = (Vec2(input.m_svDispatchThreadId.xy) + 0.5) / Vec2(kViewport);
#else
	const Vec2 uv = input.m_uv;
#endif

	RF32 weight = 1.0 / 5.0;
	RVec3 color = g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, +1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, -1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, +1)).rgb * weight;
	color += g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, -1)).rgb * weight;

	color =
		tonemap(color, readExposureAndAverageLuminance().y, g_pc.m_thresholdScalePad2.x) * g_pc.m_thresholdScalePad2.y;

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[input.m_svDispatchThreadId.xy] = RVec4(color, 0.0);
#else
	FragOut output;
	output.m_svTarget = color;
	return output;
#endif
}
