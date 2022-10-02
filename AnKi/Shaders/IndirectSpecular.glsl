// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator STOCHASTIC 0 1
#pragma anki mutator EXTRA_REJECTION 0 1

#include <AnKi/Shaders/LightFunctions.glsl>
#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/TonemappingFunctions.glsl>
#include <AnKi/Shaders/SsRaymarching.glsl>

layout(set = 0, binding = 0, row_major) uniform b_unis
{
	SsrUniforms u_unis;
};

layout(set = 0, binding = 1) uniform sampler u_trilinearClampSampler;
layout(set = 0, binding = 2) uniform ANKI_RP texture2D u_gbufferRt1;
layout(set = 0, binding = 3) uniform ANKI_RP texture2D u_gbufferRt2;
layout(set = 0, binding = 4) uniform texture2D u_depthRt;
layout(set = 0, binding = 5) uniform ANKI_RP texture2D u_lightBufferRt;

layout(set = 0, binding = 6) uniform ANKI_RP texture2D u_historyTex;
layout(set = 0, binding = 7) uniform texture2D u_motionVectorsTex;
layout(set = 0, binding = 8) uniform ANKI_RP texture2D u_historyLengthTex;

layout(set = 0, binding = 9) uniform sampler u_trilinearRepeatSampler;
layout(set = 0, binding = 10) uniform ANKI_RP texture2D u_noiseTex;
const Vec2 NOISE_TEX_SIZE = Vec2(64.0);

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 11u
#define CLUSTERED_SHADING_REFLECTIONS_BINDING 12u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 14u
#include <AnKi/Shaders/ClusteredShadingCommon.glsl>

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 kWorkgroupSize = UVec2(8, 8);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y, local_size_z = 1) in;

layout(set = 0, binding = 15) uniform writeonly image2D u_outImg;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;
#endif

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, u_unis.m_framebufferSize))
	{
		return;
	}
	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(u_unis.m_framebufferSize);
#else
	const Vec2 uv = in_uv;
#endif

	// Read part of the G-buffer
	const F32 roughness = unpackRoughnessFromGBuffer(textureLod(u_gbufferRt1, u_trilinearClampSampler, uv, 0.0));
	const Vec3 worldNormal = unpackNormalFromGBuffer(textureLod(u_gbufferRt2, u_trilinearClampSampler, uv, 0.0));

	// Get depth
	const F32 depth = textureLod(u_depthRt, u_trilinearClampSampler, uv, 0.0).r;

	// Rand idx
	const Vec2 noiseUv = Vec2(u_unis.m_framebufferSize) / NOISE_TEX_SIZE * uv;
	const Vec3 noise =
		animateBlueNoise(textureLod(u_noiseTex, u_trilinearRepeatSampler, noiseUv, 0.0).rgb, u_unis.m_frameCount % 8u);

	// Get view pos
	const Vec4 viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(uv), depth, 1.0);
	const Vec3 viewPos = viewPos4.xyz / viewPos4.w;

	// Compute refl vector
	const Vec3 viewDir = -normalize(viewPos);
	const Vec3 viewNormal = u_unis.m_normalMat * Vec4(worldNormal, 0.0);
#if STOCHASTIC
	const Vec3 reflDir = sampleReflectionVector(viewDir, viewNormal, roughness, noise.xy);
#else
	const Vec3 reflDir = reflect(-viewDir, viewNormal);
#endif

	// Is rough enough to deserve SSR?
	F32 ssrAttenuation = saturate(1.0f - pow(roughness / u_unis.m_roughnessCutoff, 16.0f));

	// Do the heavy work
	Vec3 hitPoint;
	if(ssrAttenuation > kEpsilonf)
	{
		const U32 lod = 8u; // Use the max LOD for ray marching
		const U32 step = u_unis.m_firstStepPixels;
		const F32 stepf = F32(step);
		const F32 minStepf = stepf / 4.0;
		F32 hitAttenuation;
		raymarchGroundTruth(viewPos, reflDir, uv, depth, u_unis.m_projMat, u_unis.m_maxSteps, u_depthRt,
							u_trilinearClampSampler, F32(lod), u_unis.m_depthBufferSize, step,
							U32((stepf - minStepf) * noise.x + minStepf), hitPoint, hitAttenuation);

		ssrAttenuation *= hitAttenuation;
	}
	else
	{
		ssrAttenuation = 0.0f;
	}

#if EXTRA_REJECTION
	// Reject backfacing
	ANKI_BRANCH if(ssrAttenuation > 0.0)
	{
		const Vec3 hitNormal =
			u_unis.m_normalMat
			* Vec4(unpackNormalFromGBuffer(textureLod(u_gbufferRt2, u_trilinearClampSampler, hitPoint.xy, 0.0)), 0.0);
		F32 backFaceAttenuation;
		rejectBackFaces(reflDir, hitNormal, backFaceAttenuation);

		ssrAttenuation *= backFaceAttenuation;
	}

	// Reject far from hit point
	ANKI_BRANCH if(ssrAttenuation > 0.0)
	{
		const F32 depth = textureLod(u_depthRt, u_trilinearClampSampler, hitPoint.xy, 0.0).r;
		Vec4 viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(hitPoint.xy), depth, 1.0);
		const F32 actualZ = viewPos4.z / viewPos4.w;

		viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(hitPoint.xy), hitPoint.z, 1.0);
		const F32 hitZ = viewPos4.z / viewPos4.w;

		const F32 rejectionMeters = 1.0;
		const F32 diff = abs(actualZ - hitZ);
		const F32 distAttenuation = (diff < rejectionMeters) ? 1.0 : 0.0;
		ssrAttenuation *= distAttenuation;
	}
#endif

	// Read the reflection
	Vec3 outColor = Vec3(0.0);
	ANKI_BRANCH if(ssrAttenuation > 0.0)
	{
		// Reproject the UV because you are reading the previous frame
		const Vec4 v4 = u_unis.m_prevViewProjMatMulInvViewProjMat * Vec4(UV_TO_NDC(hitPoint.xy), hitPoint.z, 1.0);
		hitPoint.xy = NDC_TO_UV(v4.xy / v4.w);

#if STOCHASTIC
		// LOD stays 0
		const F32 lod = 0.0;
#else
		// Compute the LOD based on the roughness
		const F32 lod = F32(u_unis.m_lightBufferMipCount - 1u) * roughness;
#endif

		// Read the light buffer
		Vec3 ssrColor = textureLod(u_lightBufferRt, u_trilinearClampSampler, hitPoint.xy, lod).rgb;
		ssrColor = clamp(ssrColor, 0.0, kMaxF32); // Fix the value just in case

		outColor = ssrColor;
	}

	// Blend with history
	{
		const Vec2 historyUv = uv + textureLod(u_motionVectorsTex, u_trilinearClampSampler, uv, 0.0).xy;
		const F32 historyLength = textureLod(u_historyLengthTex, u_trilinearClampSampler, uv, 0.0).x;

		const F32 lowestBlendFactor = 0.2;
		const F32 maxHistoryLength = 16.0;
		const F32 stableFrames = 4.0;
		const F32 lerp = min(1.0, (historyLength * maxHistoryLength - 1.0) / stableFrames);
		const F32 blendFactor = mix(1.0, lowestBlendFactor, lerp);

		// Blend with history
		if(blendFactor < 1.0)
		{
			const ANKI_RP Vec3 history = textureLod(u_historyTex, u_trilinearClampSampler, historyUv, 0.0).xyz;
			outColor = mix(history, outColor, blendFactor);
		}
	}

	// Read probes
	ANKI_BRANCH if(ssrAttenuation < 1.0)
	{
#if defined(ANKI_COMPUTE_SHADER)
		const Vec2 fragCoord = Vec2(gl_GlobalInvocationID.xy) + 0.5;
#else
		const Vec2 fragCoord = gl_FragCoord.xy;
#endif

#if STOCHASTIC
		const F32 reflLod = 0.0;
#else
		const F32 reflLod = (u_clusteredShading.m_reflectionProbesMipCount - 1.0) * roughness;
#endif

		// Get cluster
		const Vec2 ndc = UV_TO_NDC(uv);
		const Vec4 worldPos4 = u_clusteredShading.m_matrices.m_invertedViewProjectionJitter * Vec4(ndc, depth, 1.0);
		const Vec3 worldPos = worldPos4.xyz / worldPos4.w;
		Cluster cluster = getClusterFragCoord(Vec3(fragCoord * 2.0, depth));

		// Compute the refl dir in word space this time
		const ANKI_RP Vec3 viewDir = normalize(u_clusteredShading.m_cameraPosition - worldPos);
#if STOCHASTIC
		const Vec3 reflDir = sampleReflectionVector(viewDir, worldNormal, roughness, noise.xy);
#else
		const Vec3 reflDir = reflect(-viewDir, worldNormal);
#endif

		Vec3 probeColor = Vec3(0.0);

		if(bitCount(cluster.m_reflectionProbesMask) == 1)
		{
			// Only one probe, do a fast path without blend weight

			const ReflectionProbe probe = u_reflectionProbes[findLSB2(cluster.m_reflectionProbesMask)];

			// Sample
			const Vec3 cubeUv = intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
			const Vec4 cubeArrUv = Vec4(cubeUv, probe.m_cubemapIndex);
			probeColor = textureLod(u_reflectionsTex, u_trilinearClampSampler, cubeArrUv, reflLod).rgb;
		}
		else
		{
			// Zero or more than one probes, do a slow path that blends them together

			F32 totalBlendWeight = kEpsilonf;

			// Loop probes
			ANKI_LOOP while(cluster.m_reflectionProbesMask != 0u)
			{
				const U32 idx = U32(findLSB2(cluster.m_reflectionProbesMask));
				cluster.m_reflectionProbesMask &= ~(1u << idx);
				const ReflectionProbe probe = u_reflectionProbes[idx];

				// Compute blend weight
				const F32 blendWeight = computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, 0.2);
				totalBlendWeight += blendWeight;

				// Sample reflections
				const Vec3 cubeUv =
					intersectProbe(worldPos, reflDir, probe.m_aabbMin, probe.m_aabbMax, probe.m_position);
				const Vec4 cubeArrUv = Vec4(cubeUv, probe.m_cubemapIndex);
				const Vec3 c = textureLod(u_reflectionsTex, u_trilinearClampSampler, cubeArrUv, reflLod).rgb;
				probeColor += c * blendWeight;
			}

			// Normalize the colors
			probeColor /= totalBlendWeight;
		}

		outColor = mix(probeColor, outColor, ssrAttenuation);
	}

	// Store
	ssrAttenuation = saturate(ssrAttenuation);
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), Vec4(outColor, 0.0));
#else
	out_color = outColor;
#endif
}
