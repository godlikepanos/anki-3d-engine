// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Does SSGI and GI probe sampling

ANKI_SPECIALIZATION_CONSTANT_U32(SAMPLE_COUNT, 6u);

#define ENABLE_SSGI true
#define ENABLE_PROBES true
#define REMOVE_FIREFLIES false
#define REPROJECT_LIGHTBUFFER false
#define SSGI_PROBE_COMBINE(ssgiColor, probeColor) ((ssgiColor) + (probeColor))

#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/ImportanceSampling.glsl>
#include <AnKi/Shaders/TonemappingFunctions.glsl>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 0u
#define CLUSTERED_SHADING_GI_BINDING 1u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 3u
#include <AnKi/Shaders/ClusteredShadingCommon.glsl>

layout(set = 0, binding = 4) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 5) uniform ANKI_RP texture2D u_gbufferRt2;
layout(set = 0, binding = 6) uniform texture2D u_depthRt;
layout(set = 0, binding = 7) uniform ANKI_RP texture2D u_lightBufferRt;
layout(set = 0, binding = 8) uniform ANKI_RP texture2D u_historyTex;
layout(set = 0, binding = 9) uniform texture2D u_motionVectorsTex;
layout(set = 0, binding = 10) uniform ANKI_RP texture2D u_historyLengthTex;

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 kWorkgroupSize = UVec2(8, 8);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y) in;

layout(set = 0, binding = 11) writeonly uniform image2D u_outImage;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;
#endif

layout(push_constant, std140) uniform b_pc
{
	IndirectDiffuseUniforms u_unis;
};

Vec4 cheapProject(Vec4 point)
{
	return projectPerspective(point, u_unis.m_projectionMat.x, u_unis.m_projectionMat.y, u_unis.m_projectionMat.z,
							  u_unis.m_projectionMat.w);
}

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(gl_GlobalInvocationID.x >= u_unis.m_viewportSize.x || gl_GlobalInvocationID.y >= u_unis.m_viewportSize.y)
	{
		return;
	}

	const Vec2 fragCoord = Vec2(gl_GlobalInvocationID.xy) + 0.5;
	const Vec2 uv = fragCoord / u_unis.m_viewportSizef;
#else
	const Vec2 fragCoord = gl_FragCoord.xy;
	const Vec2 uv = in_uv;
#endif

	const Vec2 ndc = UV_TO_NDC(uv);

	// Get normal
	const Vec3 worldNormal = unpackNormalFromGBuffer(textureLod(u_gbufferRt2, u_linearAnyClampSampler, uv, 0.0));
	const Vec3 viewNormal = (u_clusteredShading.m_matrices.m_view * Vec4(worldNormal, 0.0)).xyz;

	// Get origin
	const F32 depth = textureLod(u_depthRt, u_linearAnyClampSampler, uv, 0.0).r;
	Vec4 v4 = u_clusteredShading.m_matrices.m_invertedViewProjectionJitter * Vec4(ndc, depth, 1.0);
	const Vec3 worldPos = v4.xyz / v4.w;
	v4 = u_clusteredShading.m_matrices.m_invertedProjectionJitter * Vec4(ndc, depth, 1.0);
	const Vec3 viewPos = v4.xyz / v4.w;

	// SSGI
	ANKI_RP Vec3 outColor = Vec3(0.0);
	ANKI_RP F32 ssao = 0.0;
	if(ENABLE_SSGI)
	{
		// Find the projected radius
		const ANKI_RP Vec3 sphereLimit = viewPos + Vec3(u_unis.m_radius, 0.0, 0.0);
		const ANKI_RP Vec4 projSphereLimit = cheapProject(Vec4(sphereLimit, 1.0));
		const ANKI_RP Vec2 projSphereLimit2 = projSphereLimit.xy / projSphereLimit.w;
		const ANKI_RP F32 projRadius = length(projSphereLimit2 - ndc);

		// Loop to compute
#if defined(ANKI_COMPUTE_SHADER)
		const UVec2 globalInvocation = gl_GlobalInvocationID.xy;
#else
		const UVec2 globalInvocation = UVec2(gl_FragCoord.xy);
#endif
		const UVec2 random = rand3DPCG16(UVec3(globalInvocation, u_clusteredShading.m_frame)).xy;
		const F32 aspectRatio = u_unis.m_viewportSizef.x / u_unis.m_viewportSizef.y;
		for(U32 i = 0u; i < u_unis.m_sampleCount; ++i)
		{
			const Vec2 point = UV_TO_NDC(hammersleyRandom16(i, u_unis.m_sampleCount, random)) * Vec2(1.0, aspectRatio);
			const Vec2 finalDiskPoint = ndc + point * projRadius;

			// Do a cheap unproject in view space
			const F32 d = textureLod(u_depthRt, u_linearAnyClampSampler, NDC_TO_UV(finalDiskPoint), 0.0).r;
			const F32 z = u_clusteredShading.m_matrices.m_unprojectionParameters.z
						  / (u_clusteredShading.m_matrices.m_unprojectionParameters.w + d);
			const Vec2 xy = finalDiskPoint * u_clusteredShading.m_matrices.m_unprojectionParameters.xy * z;
			const Vec3 s = Vec3(xy, z);

			// Compute factor
			const Vec3 dir = s - viewPos;
			const F32 len = length(dir);
			const Vec3 n = normalize(dir);
			const F32 NoL = max(0.0, dot(viewNormal, n));
			// const F32 distFactor = 1.0 - sin(min(1.0, len / u_unis.m_radius) * kPi / 2.0);
			const F32 distFactor = 1.0 - min(1.0, len / u_unis.m_radius);

			// Compute the UV for sampling the pyramid
			const Vec2 crntFrameUv = NDC_TO_UV(finalDiskPoint);
			Vec2 lastFrameUv;
			if(REPROJECT_LIGHTBUFFER)
			{
				lastFrameUv =
					crntFrameUv + textureLod(u_motionVectorsTex, u_linearAnyClampSampler, crntFrameUv, 0.0).xy;
			}
			else
			{
				lastFrameUv = crntFrameUv;
			}

			// Append color
			const F32 w = distFactor * NoL;
			const ANKI_RP Vec3 c = textureLod(u_lightBufferRt, u_linearAnyClampSampler, lastFrameUv, 100.0).xyz;
			outColor += c * w;

			// Compute SSAO as well
			ssao += max(dot(viewNormal, dir) + u_unis.m_ssaoBias, kEpsilonf) / max(len * len, kEpsilonf);
		}

		const ANKI_RP F32 scount = 1.0 / u_unis.m_sampleCountf;
		outColor *= scount * 2.0 * kPi;
		ssao *= scount;
	}

	ssao = min(1.0, 1.0 - ssao * u_unis.m_ssaoStrength);

	if(ENABLE_PROBES)
	{
		// Sample probes

		ANKI_RP Vec3 probeColor = Vec3(0.0);

		// Get the cluster
		Cluster cluster = getClusterFragCoord(Vec3(fragCoord * 2.0, depth));

		if(bitCount(cluster.m_giProbesMask) == 1)
		{
			// All subgroups point to the same probe and there is only one probe, do a fast path without blend weight

			const GlobalIlluminationProbe probe = u_giProbes[findLSB2(cluster.m_giProbesMask)];

			// Sample
			probeColor = sampleGlobalIllumination(worldPos, worldNormal, probe, u_globalIlluminationTextures,
												  u_linearAnyClampSampler);
		}
		else
		{
			// Zero or more than one probes, do a slow path that blends them together

			F32 totalBlendWeight = kEpsilonf;

			// Loop probes
			ANKI_LOOP while(cluster.m_giProbesMask != 0u)
			{
				const U32 idx = U32(findLSB2(cluster.m_giProbesMask));
				cluster.m_giProbesMask &= ~(1u << idx);
				const GlobalIlluminationProbe probe = u_giProbes[idx];

				// Compute blend weight
				const F32 blendWeight =
					computeProbeBlendWeight(worldPos, probe.m_aabbMin, probe.m_aabbMax, probe.m_fadeDistance);
				totalBlendWeight += blendWeight;

				// Sample
				const ANKI_RP Vec3 c = sampleGlobalIllumination(worldPos, worldNormal, probe,
																u_globalIlluminationTextures, u_linearAnyClampSampler);
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
		const F32 averageLum = (subgroupAdd(lum) / F32(gl_SubgroupSize)) * 2.0;
		const F32 newLum = min(lum, averageLum);
		outColor *= newLum / lum;
	}

	// Apply SSAO
	outColor *= ssao;

	// Blend color with history
	{
		const Vec2 historyUv = uv + textureLod(u_motionVectorsTex, u_linearAnyClampSampler, uv, 0.0).xy;
		const F32 historyLength = textureLod(u_historyLengthTex, u_linearAnyClampSampler, uv, 0.0).x;

		const F32 lowestBlendFactor = 0.05;
		const F32 maxHistoryLength = 16.0;
		const F32 stableFrames = 4.0;
		const F32 lerp = min(1.0, (historyLength * maxHistoryLength - 1.0) / stableFrames);
		const F32 blendFactor = mix(1.0, lowestBlendFactor, lerp);

		// Blend with history
		if(blendFactor < 1.0)
		{
			const ANKI_RP Vec3 history = textureLod(u_historyTex, u_linearAnyClampSampler, historyUv, 0.0).rgb;
			outColor = mix(history, outColor, blendFactor);
		}
	}

	// Store color
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImage, IVec2(gl_GlobalInvocationID.xy), Vec4(outColor, 1.0));
#else
	out_color = outColor;
#endif
}
