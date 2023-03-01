// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator PCF 0 1

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 0u
#define CLUSTERED_SHADING_LIGHTS_BINDING 1u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 4u
#include <AnKi/Shaders/ClusteredShadingCommon.hlsl>

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 0u);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(kTileCount, 2u);
ANKI_SPECIALIZATION_CONSTANT_U32(kZSplitCount, 4u);
ANKI_SPECIALIZATION_CONSTANT_U32(kTileSize, 5u);

#define DEBUG_CASCADES 0

[[vk::binding(5)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(6)]] SamplerComparisonState g_linearAnyClampShadowSampler;
[[vk::binding(7)]] SamplerState g_trilinearRepeatSampler;
[[vk::binding(8)]] Texture2D g_depthRt;
[[vk::binding(9)]] Texture2D g_noiseTex;

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(10)]] RWTexture2D<RVec4> g_outUav;
#endif

Vec3 computeDebugShadowCascadeColor(U32 cascade)
{
	if(cascade == 0u)
	{
		return Vec3(0.0f, 1.0f, 0.0f);
	}
	else if(cascade == 1u)
	{
		return Vec3(0.0f, 0.0f, 1.0f);
	}
	else if(cascade == 2u)
	{
		return Vec3(0.0f, 1.0f, 1.0f);
	}
	else
	{
		return Vec3(1.0f, 0.0f, 0.0f);
	}
}

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
RVec4 main(Vec2 uv : TEXCOORD) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(any(svDispatchThreadId.xy >= kFramebufferSize))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(kFramebufferSize);
#endif

#if PCF
	// Noise
	const Vec2 kNoiseTexSize = 64.0f;
	const Vec2 noiseUv = Vec2(kFramebufferSize) / kNoiseTexSize * uv;
	RVec3 noise = g_noiseTex.SampleLevel(g_trilinearRepeatSampler, noiseUv, 0.0).rgb;
	noise = animateBlueNoise(noise, g_clusteredShading.m_frame % 16u);
	const RF32 randFactor = noise.x;
#endif

	// World position
	const Vec2 ndc = uvToNdc(uv);
	const F32 depth = g_depthRt.SampleLevel(g_linearAnyClampSampler, uv, 0.0).r;
	const Vec4 worldPos4 = mul(g_clusteredShading.m_matrices.m_invertedViewProjectionJitter, Vec4(ndc, depth, 1.0));
	const Vec3 worldPos = worldPos4.xyz / worldPos4.w;

	// Cluster
	const Vec2 fragCoord = uv * g_clusteredShading.m_renderingSize;
	Cluster cluster = getClusterFragCoord(Vec3(fragCoord, depth), kTileSize, kTileCount, kZSplitCount,
										  g_clusteredShading.m_zSplitMagic.x, g_clusteredShading.m_zSplitMagic.y);

	// Layers
	U32 shadowCasterCountPerFragment = 0u;
	const U32 kMaxShadowCastersPerFragment = 4u;
	RVec4 shadowFactors = 0.0f;

	// Dir light
	const DirectionalLight dirLight = g_clusteredShading.m_directionalLight;
	if(dirLight.m_active != 0u && dirLight.m_shadowCascadeCount > 0u)
	{
		const RF32 positiveZViewSpace =
			testPlanePoint(g_clusteredShading.m_nearPlaneWSpace.xyz, g_clusteredShading.m_nearPlaneWSpace.w, worldPos)
			+ g_clusteredShading.m_near;

		const F32 lastCascadeDistance = dirLight.m_shadowCascadeDistances[dirLight.m_shadowCascadeCount - 1u];
		RF32 shadowFactor;
		if(positiveZViewSpace < lastCascadeDistance)
		{
			RF32 cascadeBlendFactor;
			const UVec2 cascadeIndices =
				computeShadowCascadeIndex2(positiveZViewSpace, dirLight.m_shadowCascadeDistances,
										   dirLight.m_shadowCascadeCount, cascadeBlendFactor);

#if DEBUG_CASCADES
			const Vec3 debugColorA = computeDebugShadowCascadeColor(cascadeIndices[0]);
			const Vec3 debugColorB = computeDebugShadowCascadeColor(cascadeIndices[1]);
			const Vec3 debugColor = lerp(debugColorA, debugColorB, cascadeBlendFactor);
#	if defined(ANKI_COMPUTE_SHADER)
			g_outUav[svDispatchThreadId.xy] = shadowFactors;
			return;
#	else
			return shadowFactors;
#	endif
#endif

#if PCF
			const F32 shadowFactorCascadeA = computeShadowFactorDirLightPcf(
				dirLight, cascadeIndices.x, worldPos, g_shadowAtlasTex, g_linearAnyClampShadowSampler, randFactor);
#else
			const F32 shadowFactorCascadeA = computeShadowFactorDirLight(
				dirLight, cascadeIndices.x, worldPos, g_shadowAtlasTex, g_linearAnyClampShadowSampler);
#endif

			if(cascadeBlendFactor < 0.01 || cascadeIndices.x == cascadeIndices.y)
			{
				// Don't blend cascades
				shadowFactor = shadowFactorCascadeA;
			}
			else
			{
#if PCF
				// Blend cascades
				const F32 shadowFactorCascadeB = computeShadowFactorDirLightPcf(
					dirLight, cascadeIndices.y, worldPos, g_shadowAtlasTex, g_linearAnyClampShadowSampler, randFactor);
#else
				// Blend cascades
				const F32 shadowFactorCascadeB = computeShadowFactorDirLight(
					dirLight, cascadeIndices.y, worldPos, g_shadowAtlasTex, g_linearAnyClampShadowSampler);
#endif
				shadowFactor = lerp(shadowFactorCascadeA, shadowFactorCascadeB, cascadeBlendFactor);
			}

			RF32 distanceFadeFactor = saturate(positiveZViewSpace / lastCascadeDistance);
			distanceFadeFactor = pow(distanceFadeFactor, 8.0);
			shadowFactor += distanceFadeFactor;
		}
		else
		{
			shadowFactor = 1.0;
		}

		shadowFactors[0] = shadowFactor;
		++shadowCasterCountPerFragment;
	}

	// Point lights
	[loop] while(cluster.m_pointLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = firstbitlow2(cluster.m_pointLightsMask);
		cluster.m_pointLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const PointLight light = g_pointLights[idx];

		[branch] if(light.m_shadowAtlasTileScale >= 0.0)
		{
			const Vec3 frag2Light = light.m_position - worldPos;

#if PCF
			const RF32 shadowFactor = computeShadowFactorPointLightPcf(light, frag2Light, g_shadowAtlasTex,
																	   g_linearAnyClampShadowSampler, randFactor);
#else
			const RF32 shadowFactor =
				computeShadowFactorPointLight(light, frag2Light, g_shadowAtlasTex, g_linearAnyClampShadowSampler);
#endif
			shadowFactors[min(kMaxShadowCastersPerFragment - 1u, shadowCasterCountPerFragment++)] = shadowFactor;
		}
	}

	// Spot lights
	[loop] while(cluster.m_spotLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = firstbitlow2(cluster.m_spotLightsMask);
		cluster.m_spotLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const SpotLight light = g_spotLights[idx];

		[branch] if(light.m_shadowLayer != kMaxU32)
		{
#if PCF
			const RF32 shadowFactor = computeShadowFactorSpotLightPcf(light, worldPos, g_shadowAtlasTex,
																	  g_linearAnyClampShadowSampler, randFactor);
#else
			const RF32 shadowFactor =
				computeShadowFactorSpotLight(light, worldPos, g_shadowAtlasTex, g_linearAnyClampShadowSampler);
#endif
			shadowFactors[min(kMaxShadowCastersPerFragment - 1u, shadowCasterCountPerFragment++)] = shadowFactor;
		}
	}

	// Store
#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = shadowFactors;
#else
	return shadowFactors;
#endif
}
