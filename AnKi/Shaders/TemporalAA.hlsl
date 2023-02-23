// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator VARIANCE_CLIPPING 0 1
#pragma anki mutator YCBCR 0 1

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/TonemappingFunctions.hlsl>

ANKI_SPECIALIZATION_CONSTANT_F32(kVarianceClippingGamma, 0u);
ANKI_SPECIALIZATION_CONSTANT_F32(kBlendFactor, 1u);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 2u);

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_inputRt;
[[vk::binding(2)]] Texture2D<RVec4> g_historyRt;
[[vk::binding(3)]] Texture2D g_motionVectorsTex;

constexpr U32 kTonemappingBinding = 4u;
#include <AnKi/Shaders/TonemappingResources.hlsl>

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(5)]] RWTexture2D<RVec4> g_outImg;
[[vk::binding(6)]] RWTexture2D<RVec4> g_tonemappedImg;
#else
struct FragOut
{
	RVec3 m_color : SV_TARGET0;
	RVec3 m_tonemappedColor : SV_TARGET1;
};
#endif

#if YCBCR
#	define sample(s, uv) rgbToYCbCr(s.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb)
#	define sampleOffset(s, uv, x, y) rgbToYCbCr(s.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(x, y)).rgb)
#else
#	define sample(s, uv) s.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb
#	define sampleOffset(s, uv, x, y) s.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(x, y)).rgb
#endif

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
FragOut main(Vec2 uv : TEXCOORD)
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(any(svDispatchThreadId.xy >= kFramebufferSize))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(kFramebufferSize);
#endif

	// Get prev uv coords
	const Vec2 oldUv = uv + g_motionVectorsTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rg;

	// Read textures
	Vec3 historyCol = sample(g_historyRt, oldUv);
	const Vec3 crntCol = sample(g_inputRt, uv);

	// Remove ghosting by clamping the history color to neighbour's AABB
	const Vec3 near0 = sampleOffset(g_inputRt, uv, 1, 0);
	const Vec3 near1 = sampleOffset(g_inputRt, uv, 0, 1);
	const Vec3 near2 = sampleOffset(g_inputRt, uv, -1, 0);
	const Vec3 near3 = sampleOffset(g_inputRt, uv, 0, -1);

#if VARIANCE_CLIPPING
	const Vec3 m1 = crntCol + near0 + near1 + near2 + near3;
	const Vec3 m2 = crntCol * crntCol + near0 * near0 + near1 * near1 + near2 * near2 + near3 * near3;

	const Vec3 mu = m1 / 5.0;
	const Vec3 sigma = sqrt(m2 / 5.0 - mu * mu);

	const Vec3 boxMin = mu - kVarianceClippingGamma * sigma;
	const Vec3 boxMax = mu + kVarianceClippingGamma * sigma;
#else
	const Vec3 boxMin = min(crntCol, min(near0, min(near1, min(near2, near3))));
	const Vec3 boxMax = max(crntCol, max(near0, max(near1, max(near2, near3))));
#endif

	historyCol = clamp(historyCol, boxMin, boxMax);

	// Remove jitter (T. Lottes)
#if YCBCR
	const F32 lum0 = crntCol.r;
	const F32 lum1 = historyCol.r;
	const F32 maxLum = boxMax.r;
#else
	const F32 lum0 = computeLuminance(reinhardTonemap(crntCol));
	const F32 lum1 = computeLuminance(reinhardTonemap(historyCol));
	const F32 maxLum = 1.0;
#endif

	F32 diff = abs(lum0 - lum1) / max(lum0, max(lum1, maxLum + kEpsilonF32));
	diff = 1.0 - diff;
	diff = diff * diff;
	const F32 feedback = lerp(0.0, kBlendFactor, diff);

	// Write result
	Vec3 outColor = lerp(historyCol, crntCol, feedback);
#if YCBCR
	outColor = yCbCrToRgb(outColor);
#endif
	const Vec3 tonemapped = linearToSRgb(tonemap(outColor, readExposureAndAverageLuminance().x));
#if defined(ANKI_COMPUTE_SHADER)
	g_outImg[svDispatchThreadId.xy] = RVec4(outColor, 0.0);
	g_tonemappedImg[svDispatchThreadId.xy] = RVec4(tonemapped, 0.0);
#else
	FragOut output;
	output.m_color = outColor;
	output.m_tonemappedColor = tonemapped;
	return output;
#endif
}
