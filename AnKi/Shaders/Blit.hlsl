// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Functions.hlsl>

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_inputTex;

#if defined(ANKI_COMPUTE_SHADER)
#	define USE_COMPUTE 1
#else
#	define USE_COMPUTE 0
#endif

#if USE_COMPUTE
[[vk::binding(2)]] RWTexture2D<RVec4> g_outUav;

struct Uniforms
{
	Vec2 m_viewportSize;
	UVec2 m_viewportSizeU;
};

[[vk::push_constant]] ConstantBuffer<Uniforms> g_pc;
#endif

#if USE_COMPUTE
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if USE_COMPUTE
	if(skipOutOfBoundsInvocations(UVec2(8, 8), g_pc.m_viewportSizeU, svDispatchThreadId.xy))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / g_pc.m_viewportSize;
#endif

	const RVec3 color = g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb;

#if USE_COMPUTE
	g_outUav[svDispatchThreadId.xy] = RVec4(color, 0.0);
#else
	return color;
#endif
}
