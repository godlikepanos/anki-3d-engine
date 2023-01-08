// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Does SSGI and GI probe sampling

#pragma anki hlsl

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/ImportanceSampling.hlsl>
#include <AnKi/Shaders/TonemappingFunctions.hlsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

#define ENABLE_SSGI false
#define ENABLE_PROBES true
#define REMOVE_FIREFLIES false
#define REPROJECT_LIGHTBUFFER false
#define SSGI_PROBE_COMBINE(ssgiColor, probeColor) ((ssgiColor) + (probeColor))

ANKI_SPECIALIZATION_CONSTANT_U32(kSampleCount, 0u);

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 0u
#define CLUSTERED_SHADING_GI_BINDING 1u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 3u
#include <AnKi/Shaders/ClusteredShadingCommon.hlsl>

[[vk::binding(4)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(5)]] Texture2D<RVec4> g_gbufferRt2;
[[vk::binding(6)]] Texture2D g_depthTex;
[[vk::binding(7)]] Texture2D<RVec4> g_lightBufferRt;
[[vk::binding(8)]] Texture2D<RVec4> g_historyTex;
[[vk::binding(9)]] Texture2D g_motionVectorsTex;
[[vk::binding(10)]] Texture2D g_historyLengthTex;

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(11)]] RWTexture2D<RVec3> g_outUav;
#endif

[[vk::push_constant]] ConstantBuffer<IndirectDiffuseUniforms> g_uniforms;

Vec4 cheapProject(Vec4 point_)
{
	return projectPerspective(point_, g_uniforms.m_projectionMat.x, g_uniforms.m_projectionMat.y,
							  g_uniforms.m_projectionMat.z, g_uniforms.m_projectionMat.w);
}

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec3 main([[vk::location(0)]] Vec2 uv : TEXCOORD, Vec4 svPosition : SV_POSITION) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(svDispatchThreadId.x >= g_uniforms.m_viewportSize.x || svDispatchThreadId.y >= g_uniforms.m_viewportSize.y)
	{
		return;
	}

	const Vec2 fragCoord = Vec2(svDispatchThreadId.xy) + 0.5;
	const Vec2 uv = fragCoord / g_uniforms.m_viewportSizef;
#else
	const Vec2 fragCoord = svPosition.xy;
#endif

	const Vec2 ndc = uvToNdc(uv);

	// Get normal
	const Vec3 worldNormal = unpackNormalFromGBuffer(g_gbufferRt2.SampleLevel(g_linearAnyClampSampler, uv, 0.0));
	const Vec3 viewNormal = mul(g_clusteredShading.m_matrices.m_view, Vec4(worldNormal, 0.0));

	// Get origin
	const F32 depth = g_depthTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).r;
	Vec4 v4 = mul(g_clusteredShading.m_matrices.m_invertedViewProjectionJitter, Vec4(ndc, depth, 1.0));
	const Vec3 worldPos = v4.xyz / v4.w;
	v4 = mul(g_clusteredShading.m_matrices.m_invertedProjectionJitter, Vec4(ndc, depth, 1.0));
	const Vec3 viewPos = v4.xyz / v4.w;

	// SSGI
	RVec3 outColor = Vec3(0.0, 0.0, 0.0);
	RF32 ssao = 0.0;
	if(ENABLE_SSGI)
	{
		// Find the projected radius
		const RVec3 sphereLimit = viewPos + Vec3(g_uniforms.m_radius, 0.0, 0.0);
		const RVec4 projSphereLimit = cheapProject(Vec4(sphereLimit, 1.0));
		const RVec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
		const RF32 projRadius = length(projSphereLimit2 - ndc);

		// Loop to compute
#if defined(ANKI_COMPUTE_SHADER)
		const UVec2 globalInvocation = svDispatchThreadId.xy;
#else
		const UVec2 globalInvocation = UVec2(svPosition.xy);
#endif
		const UVec2 random = rand3DPCG16(UVec3(globalInvocation, g_clusteredShading.m_frame)).xy;
		const F32 aspectRatio = g_uniforms.m_viewportSizef.x / g_uniforms.m_viewportSizef.y;
		for(U32 i = 0u; i < g_uniforms.m_sampleCount; ++i)
		{
			const Vec2 point_ =
				uvToNdc(hammersleyRandom16(i, g_uniforms.m_sampleCount, random)) * Vec2(1.0, aspectRatio);
			const Vec2 finalDiskPoint = ndc + point_ * projRadius;

			// Do a cheap unproject in view space
			const F32 d = g_depthTex.SampleLevel(g_linearAnyClampSampler, ndcToUv(finalDiskPoint), 0.0).r;
			const F32 z = g_clusteredShading.m_matrices.m_unprojectionParameters.z
						  / (g_clusteredShading.m_matrices.m_unprojectionParameters.w + d);
			const Vec2 xy = finalDiskPoint * g_clusteredShading.m_matrices.m_unprojectionParameters.xy * z;
			const Vec3 s = Vec3(xy, z);

			// Compute factor
			const Vec3 dir = s - viewPos;
			const F32 len = length(dir);
			const Vec3 n = normalize(dir);
			const F32 NoL = max(0.0, dot(viewNormal, n));
			// const F32 distFactor = 1.0 - sin(min(1.0, len / g_uniforms.m_radius) * kPi / 2.0);
			const F32 distFactor = 1.0 - min(1.0, len / g_uniforms.m_radius);

			// Compute the UV for sampling the pyramid
			const Vec2 crntFrameUv = ndcToUv(finalDiskPoint);
			Vec2 lastFrameUv;
			if(REPROJECT_LIGHTBUFFER)
			{
				lastFrameUv =
					crntFrameUv + g_motionVectorsTex.SampleLevel(g_linearAnyClampSampler, crntFrameUv, 0.0).xy;
			}
			else
			{
				lastFrameUv = crntFrameUv;
			}

			// Append color
			const F32 w = distFactor * NoL;
			const RVec3 c = g_lightBufferRt.SampleLevel(g_linearAnyClampSampler, lastFrameUv, 100.0).xyz;
			outColor += c * w;

			// Compute SSAO as well
			ssao += max(dot(viewNormal, dir) + g_uniforms.m_ssaoBias, kEpsilonF32) / max(len * len, kEpsilonF32);
		}

		const RF32 scount = 1.0 / g_uniforms.m_sampleCountf;
		outColor *= scount * 2.0 * kPi;
		ssao *= scount;
	}

	ssao = min(1.0, 1.0 - ssao * g_uniforms.m_ssaoStrength);

	if(ENABLE_PROBES)
	{
		// Sample probes

		RVec3 probeColor = Vec3(0.0, 0.0, 0.0);

		// Get the cluster
		Cluster cluster = getClusterFragCoord(Vec3(fragCoord * 2.0, depth));

		if(countbits(cluster.m_giProbesMask) == 1)
		{
			// All subgroups point to the same probe and there is only one probe, do a fast path without blend weight

			const GlobalIlluminationProbe probe = g_giProbes[firstbitlow2(cluster.m_giProbesMask)];

			// Sample
			probeColor = sampleGlobalIllumination(worldPos, worldNormal, probe, g_globalIlluminationTextures,
												  g_linearAnyClampSampler);
		}
		else
		{
			// Zero or more than one probes, do a slow path that blends them together

			F32 totalBlendWeight = kEpsilonF32;

			// Loop probes
			[loop] while(cluster.m_giProbesMask != 0u)
			{
				const U32 idx = U32(firstbitlow2(cluster.m_giProbesMask));
				cluster.m_giProbesMask &= ~(1u << idx);
				const GlobalIlluminationProbe probe = g_giProbes[idx];

				// Compute blend weight
				const F32 blendWeight =
					computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, probe.m_fadeDistance);
				totalBlendWeight += blendWeight;

				// Sample
				const RVec3 c = sampleGlobalIllumination(worldPos, worldNormal, probe, g_globalIlluminationTextures,
														 g_linearAnyClampSampler);
				probeColor += c * blendWeight;
			}

			// Normalize
			probeColor /= totalBlendWeight;
		}

		outColor = SSGI_PROBE_COMBINE(outColor, probeColor);
	}

	// Remove fireflies
	if(REMOVE_FIREFLIES)
	{
		const F32 lum = computeLuminance(outColor) + 0.001;
		const F32 averageLum = (WaveActiveSum(lum) / F32(WaveGetLaneCount())) * 2.0;
		const F32 newLum = min(lum, averageLum);
		outColor *= newLum / lum;
	}

	// Apply SSAO
	outColor *= ssao;

	// Blend color with history
	{
		const Vec2 historyUv = uv + g_motionVectorsTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).xy;
		const F32 historyLength = g_historyLengthTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).x;

		const F32 lowestBlendFactor = 0.05;
		const F32 maxHistoryLength = 16.0;
		const F32 stableFrames = 4.0;
		const F32 lerpVal = min(1.0, (historyLength * maxHistoryLength - 1.0) / stableFrames);
		const F32 blendFactor = lerp(1.0, lowestBlendFactor, lerpVal);

		// Blend with history
		if(blendFactor < 1.0)
		{
			const RVec3 history = g_historyTex.SampleLevel(g_linearAnyClampSampler, historyUv, 0.0).rgb;
			outColor = lerp(history, outColor, blendFactor);
		}
	}

	// Store color
#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = outColor;
#else
	return outColor;
#endif
}
