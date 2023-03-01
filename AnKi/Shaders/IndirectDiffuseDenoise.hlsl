// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator BLUR_ORIENTATION 0 1 // 0: in X axis, 1: in Y axis

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/BilateralFilter.hlsl>

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_toDenoiseTex;
[[vk::binding(2)]] Texture2D g_depthTex;

#if defined(ANKI_COMPUTE_SHADER)
#	define THREAD_GROUP_SIZE_SQRT 8
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#endif

[[vk::push_constant]] ConstantBuffer<IndirectDiffuseDenoiseUniforms> g_uniforms;

Vec3 unproject(Vec2 ndc, F32 depth)
{
	const Vec4 worldPos4 = mul(g_uniforms.m_invertedViewProjectionJitterMat, Vec4(ndc, depth, 1.0));
	const Vec3 worldPos = worldPos4.xyz / worldPos4.w;
	return worldPos;
}

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(THREAD_GROUP_SIZE_SQRT, THREAD_GROUP_SIZE_SQRT, 1)] void
main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(UVec2(THREAD_GROUP_SIZE_SQRT, THREAD_GROUP_SIZE_SQRT), g_uniforms.m_viewportSize,
								  svDispatchThreadId))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / g_uniforms.m_viewportSizef;
#endif

	// Reference
	const F32 depthCenter = g_depthTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).r;
	if(depthCenter == 1.0)
	{
#if defined(ANKI_COMPUTE_SHADER)
		g_outUav[svDispatchThreadId.xy] = 0.0f;
		return;
#else
		return 0.0f;
#endif
	}

	// Sample
	RF32 weight = kEpsilonRF32;
	Vec3 color = Vec3(0.0, 0.0, 0.0); // Keep it full precision because it may go to inf

	for(F32 i = -g_uniforms.m_sampleCountDiv2; i <= g_uniforms.m_sampleCountDiv2; i += 1.0)
	{
		const Vec2 texelSize = 1.0 / g_uniforms.m_viewportSizef;
#if BLUR_ORIENTATION == 0
		const Vec2 sampleUv = Vec2(uv.x + i * texelSize.x, uv.y);
#else
		const Vec2 sampleUv = Vec2(uv.x, uv.y + i * texelSize.y);
#endif

		const F32 depthTap = g_depthTex.SampleLevel(g_linearAnyClampSampler, sampleUv, 0.0).r;

		RF32 w = calculateBilateralWeightDepth(depthCenter, depthTap, 1.0);
		// w *= gaussianWeight(0.4, abs(F32(i)) / (g_uniforms.m_sampleCountDiv2 * 2.0 + 1.0));
		weight += w;

		color += g_toDenoiseTex.SampleLevel(g_linearAnyClampSampler, sampleUv, 0.0).xyz * w;
	}

	// Normalize and store
	color /= weight;

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = RVec4(color, 0.0f);
#else
	return color;
#endif
}
