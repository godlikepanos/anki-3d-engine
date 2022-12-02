// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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

struct CompIn
{
	UVec3 m_svDispatchThreadId : SV_DISPATCHTHREADID;
};
#else
struct VertOut
{
	[[vk::location(0)]] Vec2 m_uv : TEXCOORD;
	Vec4 m_svPosition : SV_POSITION;
};

struct FragOut
{
	RVec3 m_svTarget : SV_TARGET0;
};
#endif

#if USE_COMPUTE
ANKI_NUMTHREADS(8, 8, 1) void main(CompIn input)
#else
FragOut main(VertOut input)
#endif
{
#if USE_COMPUTE
	if(skipOutOfBoundsInvocations(UVec2(8, 8), g_pc.m_viewportSizeU, input.m_svDispatchThreadId.xy))
	{
		return;
	}

	const Vec2 uv = (Vec2(input.m_svDispatchThreadId.xy) + 0.5) / g_pc.m_viewportSize;
#else
	const Vec2 uv = input.m_uv;
#endif

	const RVec3 color = g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb;

#if USE_COMPUTE
	g_outUav[input.m_svDispatchThreadId.xy] = RVec4(color, 0.0);
#else
	FragOut output;
	output.m_svTarget = color;
	return output;
#endif
}
