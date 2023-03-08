// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator STOCHASTIC 0 1
#pragma anki mutator EXTRA_REJECTION 0 1

#include <AnKi/Shaders/LightFunctions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/TonemappingFunctions.hlsl>
#include <AnKi/Shaders/SsRaymarching.hlsl>

[[vk::binding(0)]] ConstantBuffer<SsrUniforms> g_unis;

[[vk::binding(1)]] SamplerState g_trilinearClampSampler;
[[vk::binding(2)]] Texture2D<RVec4> g_gbufferRt1;
[[vk::binding(3)]] Texture2D<RVec4> g_gbufferRt2;
[[vk::binding(4)]] Texture2D g_depthRt;
[[vk::binding(5)]] Texture2D<RVec4> g_lightBufferRt;

[[vk::binding(6)]] Texture2D<RVec4> g_historyTex;
[[vk::binding(7)]] Texture2D g_motionVectorsTex;
[[vk::binding(8)]] Texture2D<RVec4> g_historyLengthTex;

[[vk::binding(9)]] SamplerState g_trilinearRepeatSampler;
[[vk::binding(10)]] Texture2D<RVec4> g_noiseTex;
constexpr Vec2 kNoiseTexSize = 64.0;

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 11u
#define CLUSTERED_SHADING_REFLECTIONS_BINDING 12u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 13u
#include <AnKi/Shaders/ClusteredShadingCommon.hlsl>

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(14)]] RWTexture2D<RVec4> g_outUav;
#endif

ANKI_BINDLESS_SET(1)

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main(Vec2 uv : TEXCOORD, Vec4 svPosition : SV_POSITION) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(any(svDispatchThreadId.xy >= g_unis.m_framebufferSize))
	{
		return;
	}
	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(g_unis.m_framebufferSize);
#endif

	// Read part of the G-buffer
	const F32 roughness = unpackRoughnessFromGBuffer(g_gbufferRt1.SampleLevel(g_trilinearClampSampler, uv, 0.0));
	const Vec3 worldNormal = unpackNormalFromGBuffer(g_gbufferRt2.SampleLevel(g_trilinearClampSampler, uv, 0.0));

	// Get depth
	const F32 depth = g_depthRt.SampleLevel(g_trilinearClampSampler, uv, 0.0).r;

	// Rand idx
	const Vec2 noiseUv = Vec2(g_unis.m_framebufferSize) / kNoiseTexSize * uv;
	const Vec3 noise =
		animateBlueNoise(g_noiseTex.SampleLevel(g_trilinearRepeatSampler, noiseUv, 0.0).rgb, g_unis.m_frameCount % 8u);

	// Get view pos
	const Vec4 viewPos4 = mul(g_unis.m_invProjMat, Vec4(uvToNdc(uv), depth, 1.0));
	const Vec3 viewPos = viewPos4.xyz / viewPos4.w;

	// Compute refl vector
	const Vec3 viewDir = -normalize(viewPos);
	const Vec3 viewNormal = mul(g_unis.m_normalMat, Vec4(worldNormal, 0.0));
#if STOCHASTIC
	const Vec3 reflDir = sampleReflectionVector(viewDir, viewNormal, roughness, noise.xy);
#else
	const Vec3 reflDir = reflect(-viewDir, viewNormal);
#endif

	// Is rough enough to deserve SSR?
	F32 ssrAttenuation = saturate(1.0f - pow(roughness / g_unis.m_roughnessCutoff, 16.0f));

	// Do the heavy work
	Vec3 hitPoint;
	if(ssrAttenuation > kEpsilonF32)
	{
		const U32 lod = 8u; // Use the max LOD for ray marching
		const U32 step = g_unis.m_firstStepPixels;
		const F32 stepf = F32(step);
		const F32 minStepf = stepf / 4.0;
		F32 hitAttenuation;
		raymarchGroundTruth(viewPos, reflDir, uv, depth, g_unis.m_projMat, g_unis.m_maxSteps, g_depthRt,
							g_trilinearClampSampler, F32(lod), g_unis.m_depthBufferSize, step,
							U32((stepf - minStepf) * noise.x + minStepf), hitPoint, hitAttenuation);

		ssrAttenuation *= hitAttenuation;
	}
	else
	{
		ssrAttenuation = 0.0f;
	}

#if EXTRA_REJECTION
	// Reject backfacing
	[branch] if(ssrAttenuation > 0.0)
	{
		const Vec3 gbufferNormal =
			unpackNormalFromGBuffer(g_gbufferRt2.SampleLevel(g_trilinearClampSampler, hitPoint.xy, 0.0));
		const Vec3 hitNormal = mul(g_unis.m_normalMat, Vec4(gbufferNormal, 0.0));
		F32 backFaceAttenuation;
		rejectBackFaces(reflDir, hitNormal, backFaceAttenuation);

		ssrAttenuation *= backFaceAttenuation;
	}

	// Reject far from hit point
	[branch] if(ssrAttenuation > 0.0)
	{
		const F32 depth = g_depthRt.SampleLevel(g_trilinearClampSampler, hitPoint.xy, 0.0).r;
		Vec4 viewPos4 = mul(g_unis.m_invProjMat, Vec4(uvToNdc(hitPoint.xy), depth, 1.0));
		const F32 actualZ = viewPos4.z / viewPos4.w;

		viewPos4 = mul(g_unis.m_invProjMat, Vec4(uvToNdc(hitPoint.xy), hitPoint.z, 1.0));
		const F32 hitZ = viewPos4.z / viewPos4.w;

		const F32 rejectionMeters = 1.0;
		const F32 diff = abs(actualZ - hitZ);
		const F32 distAttenuation = (diff < rejectionMeters) ? 1.0 : 0.0;
		ssrAttenuation *= distAttenuation;
	}
#endif

	// Read the reflection
	Vec3 outColor = 0.0;
	[branch] if(ssrAttenuation > 0.0)
	{
		// Reproject the UV because you are reading the previous frame
		const Vec4 v4 = mul(g_unis.m_prevViewProjMatMulInvViewProjMat, Vec4(uvToNdc(hitPoint.xy), hitPoint.z, 1.0));
		hitPoint.xy = ndcToUv(v4.xy / v4.w);

#if STOCHASTIC
		// LOD stays 0
		const F32 lod = 0.0;
#else
		// Compute the LOD based on the roughness
		const F32 lod = F32(g_unis.m_lightBufferMipCount - 1u) * roughness;
#endif

		// Read the light buffer
		Vec3 ssrColor = g_lightBufferRt.SampleLevel(g_trilinearClampSampler, hitPoint.xy, lod).rgb;
		ssrColor = clamp(ssrColor, 0.0, kMaxF32); // Fix the value just in case

		outColor = ssrColor;
	}

	// Blend with history
	{
		const Vec2 historyUv = uv + g_motionVectorsTex.SampleLevel(g_trilinearClampSampler, uv, 0.0).xy;
		const F32 historyLength = g_historyLengthTex.SampleLevel(g_trilinearClampSampler, uv, 0.0).x;

		const F32 lowestBlendFactor = 0.2;
		const F32 maxHistoryLength = 16.0;
		const F32 stableFrames = 4.0;
		const F32 lerpv = min(1.0, (historyLength * maxHistoryLength - 1.0) / stableFrames);
		const F32 blendFactor = lerp(1.0, lowestBlendFactor, lerpv);

		// Blend with history
		if(blendFactor < 1.0)
		{
			const RVec3 history = g_historyTex.SampleLevel(g_trilinearClampSampler, historyUv, 0.0).xyz;
			outColor = lerp(history, outColor, blendFactor);
		}
	}

	// Read probes
	[branch] if(ssrAttenuation < 1.0)
	{
#if defined(ANKI_COMPUTE_SHADER)
		const Vec2 fragCoord = Vec2(svDispatchThreadId.xy) + 0.5;
#else
		const Vec2 fragCoord = svPosition.xy;
#endif

#if STOCHASTIC
		const F32 reflLod = 0.0;
#else
		const F32 reflLod = (g_clusteredShading.m_reflectionProbesMipCount - 1.0) * roughness;
#endif

		// Get cluster
		const Vec2 ndc = uvToNdc(uv);
		const Vec4 worldPos4 = mul(g_clusteredShading.m_matrices.m_invertedViewProjectionJitter, Vec4(ndc, depth, 1.0));
		const Vec3 worldPos = worldPos4.xyz / worldPos4.w;
		Cluster cluster = getClusterFragCoord(Vec3(fragCoord * 2.0, depth));

		// Compute the refl dir in word space this time
		const RVec3 viewDir = normalize(g_clusteredShading.m_cameraPosition - worldPos);
#if STOCHASTIC
		const Vec3 reflDir = sampleReflectionVector(viewDir, worldNormal, roughness, noise.xy);
#else
		const Vec3 reflDir = reflect(-viewDir, worldNormal);
#endif

		Vec3 probeColor = 0.0;

		const U32 oneProbe = WaveActiveAllTrue(countbits(cluster.m_reflectionProbesMask) == 1);
		if(oneProbe)
		{
			// Only one probe, do a fast path without blend weight

			const ReflectionProbe probe = g_reflectionProbes[firstbitlow2(cluster.m_reflectionProbesMask)];

			// Sample
			const Vec3 cubeUv = intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
			probeColor = g_bindlessTexturesCubeF32[probe.m_cubeTexture]
							 .SampleLevel(g_trilinearClampSampler, cubeUv, reflLod)
							 .rgb;
		}
		else
		{
			// Zero or more than one probes, do a slow path that blends them together

			F32 totalBlendWeight = kEpsilonF32;

			// Loop probes
			[loop] while(cluster.m_reflectionProbesMask != 0u)
			{
				const U32 idx = U32(firstbitlow2(cluster.m_reflectionProbesMask));
				cluster.m_reflectionProbesMask &= ~(1u << idx);
				const ReflectionProbe probe = g_reflectionProbes[idx];

				// Compute blend weight
				const F32 blendWeight = computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, 0.2);
				totalBlendWeight += blendWeight;

				// Sample reflections
				const Vec3 cubeUv =
					intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
				const Vec3 c = g_bindlessTexturesCubeF32[NonUniformResourceIndex(probe.m_cubeTexture)]
								   .SampleLevel(g_trilinearClampSampler, cubeUv, reflLod)
								   .rgb;
				probeColor += c * blendWeight;
			}

			// Normalize the colors
			probeColor /= totalBlendWeight;
		}

		outColor = lerp(probeColor, outColor, ssrAttenuation);
	}

	// Store
	ssrAttenuation = saturate(ssrAttenuation);
#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = RVec4(outColor, 0.0);
#else
	return outColor;
#endif
}
