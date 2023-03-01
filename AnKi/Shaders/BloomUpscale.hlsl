// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Functions.hlsl>

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kViewport, 0u);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(kInputTextureSize, 2u);

// Constants
#define ENABLE_CHROMATIC_DISTORTION 1
#define ENABLE_HALO 1
constexpr U32 kMaxGhosts = 4u;
constexpr F32 kGhostDispersal = 0.7;
constexpr F32 kHaloWidth = 0.4;
constexpr F32 kChromaticDistortion = 3.0;
constexpr F32 kHaloOpacity = 0.5;

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<RVec4> g_inputTex;
[[vk::binding(2)]] Texture2D<RVec3> g_lensDirtTex;

#if defined(ANKI_COMPUTE_SHADER)
#	define THREADGROUP_SIZE_XY 8
[[vk::binding(3)]] RWTexture2D<RVec4> g_outUav;
#endif

RVec3 textureDistorted(Texture2D<RVec4> tex, SamplerState sampl, Vec2 uv,
					   Vec2 direction, // direction of distortion
					   Vec3 distortion) // per-channel distortion factor
{
#if ENABLE_CHROMATIC_DISTORTION
	return RVec3(tex.SampleLevel(sampl, uv + direction * distortion.r, 0.0).r,
				 tex.SampleLevel(sampl, uv + direction * distortion.g, 0.0).g,
				 tex.SampleLevel(sampl, uv + direction * distortion.b, 0.0).b);
#else
	return tex.SampleLevel(uv, 0.0).rgb;
#endif
}

RVec3 ssLensFlare(Vec2 uv)
{
	constexpr Vec2 kTexelSize = 1.0 / Vec2(kInputTextureSize);
	constexpr Vec3 kDistortion = Vec3(-kTexelSize.x * kChromaticDistortion, 0.0, kTexelSize.x * kChromaticDistortion);
	constexpr F32 kLensOfHalf = length(Vec2(0.5, 0.5));

	const Vec2 flipUv = Vec2(1.0, 1.0) - uv;

	const Vec2 ghostVec = (Vec2(0.5, 0.5) - flipUv) * kGhostDispersal;

	const Vec2 direction = normalize(ghostVec);
	RVec3 result = Vec3(0.0, 0.0, 0.0);

	// Sample ghosts
	[unroll] for(U32 i = 0u; i < kMaxGhosts; ++i)
	{
		const Vec2 offset = frac(flipUv + ghostVec * F32(i));

		RF32 weight = length(Vec2(0.5, 0.5) - offset) / kLensOfHalf;
		weight = pow(1.0 - weight, 10.0);

		result += textureDistorted(g_inputTex, g_linearAnyClampSampler, offset, direction, kDistortion) * weight;
	}

	// Sample halo
#if ENABLE_HALO
	const Vec2 haloVec = normalize(ghostVec) * kHaloWidth;
	RF32 weight = length(Vec2(0.5, 0.5) - frac(flipUv + haloVec)) / kLensOfHalf;
	weight = pow(1.0 - weight, 20.0);
	result += textureDistorted(g_inputTex, g_linearAnyClampSampler, flipUv + haloVec, direction, kDistortion)
			  * (weight * kHaloOpacity);
#endif

	// Lens dirt
	result *= g_lensDirtTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb;

	return result;
}

RVec3 upscale(Vec2 uv)
{
	const RF32 weight = 1.0 / 5.0;
	RVec3 result = g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rgb * weight;
	result += g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, +1)).rgb * weight;
	result += g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(+1, -1)).rgb * weight;
	result += g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, -1)).rgb * weight;
	result += g_inputTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0, IVec2(-1, +1)).rgb * weight;

	return result;
}

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(THREADGROUP_SIZE_XY, THREADGROUP_SIZE_XY, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(UVec2(THREADGROUP_SIZE_XY, THREADGROUP_SIZE_XY), kViewport, svDispatchThreadId))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(kViewport);
#endif

	const RVec3 outColor = ssLensFlare(uv) + upscale(uv);

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = RVec4(outColor, 0.0);
#else
	return outColor;
#endif
}
