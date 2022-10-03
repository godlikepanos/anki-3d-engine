// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 0u);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(kTileCount, 2u);
ANKI_SPECIALIZATION_CONSTANT_U32(kZSplitCount, 4u);
ANKI_SPECIALIZATION_CONSTANT_U32(kTileSize, 5u);

#define CLUSTERED_SHADING_SET 0u
#define CLUSTERED_SHADING_UNIFORMS_BINDING 0u
#define CLUSTERED_SHADING_LIGHTS_BINDING 1u
#define CLUSTERED_SHADING_CLUSTERS_BINDING 4u
#include <AnKi/Shaders/ClusteredShadingCommon.glsl>

layout(set = 0, binding = 5) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 6) uniform texture2D u_depthRt;

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 kWorkgroupSize = UVec2(8, 8);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y, local_size_z = 1) in;
layout(set = 0, binding = 7, rgba8) writeonly uniform ANKI_RP image2D u_outImg;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec4 out_color;
#endif

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, kFramebufferSize))
	{
		return;
	}
	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(kFramebufferSize);
#else
	const Vec2 uv = in_uv;
#endif

	// World position
	const Vec2 ndc = UV_TO_NDC(uv);
	const F32 depth = textureLod(u_depthRt, u_linearAnyClampSampler, uv, 0.0).r;
	const Vec4 worldPos4 = u_clusteredShading.m_matrices.m_invertedViewProjectionJitter * Vec4(ndc, depth, 1.0);
	const Vec3 worldPos = worldPos4.xyz / worldPos4.w;

	// Cluster
	const Vec2 fragCoord = uv * u_clusteredShading.m_renderingSize;
	Cluster cluster = getClusterFragCoord(Vec3(fragCoord, depth), kTileSize, kTileCount, kZSplitCount,
										  u_clusteredShading.m_zSplitMagic.x, u_clusteredShading.m_zSplitMagic.y);

	// Layers
	U32 shadowCasterCountPerFragment = 0u;
	const U32 maxShadowCastersPerFragment = 4u;
	ANKI_RP F32 shadowFactors[maxShadowCastersPerFragment] = F32[](0.0, 0.0, 0.0, 0.0);

	// Dir light
	const DirectionalLight dirLight = u_clusteredShading.m_directionalLight;
	if(dirLight.m_active != 0u && dirLight.m_cascadeCount > 0u)
	{
		const ANKI_RP F32 positiveZViewSpace =
			testPlanePoint(u_clusteredShading.m_nearPlaneWSpace.xyz, u_clusteredShading.m_nearPlaneWSpace.w, worldPos)
			+ u_clusteredShading.m_near;

		ANKI_RP F32 shadowFactor;
		if(positiveZViewSpace < dirLight.m_effectiveShadowDistance)
		{
			F32 cascadeBlendFactor;
			const UVec2 cascadeIndices = computeShadowCascadeIndex2(
				positiveZViewSpace, dirLight.m_shadowCascadesDistancePower, dirLight.m_effectiveShadowDistance,
				dirLight.m_cascadeCount, cascadeBlendFactor);

			const F32 shadowFactorCascadeA = computeShadowFactorDirLight(dirLight, cascadeIndices.x, worldPos,
																		 u_shadowAtlasTex, u_linearAnyClampSampler);

			if(cascadeBlendFactor < 0.01 || cascadeIndices.x == cascadeIndices.y)
			{
				// Don't blend cascades
				shadowFactor = shadowFactorCascadeA;
			}
			else
			{
				// Blend cascades
				const F32 shadowFactorCascadeB = computeShadowFactorDirLight(dirLight, cascadeIndices.y, worldPos,
																			 u_shadowAtlasTex, u_linearAnyClampSampler);

				shadowFactor = mix(shadowFactorCascadeA, shadowFactorCascadeB, cascadeBlendFactor);
			}

			ANKI_RP F32 distanceFadeFactor = saturate(positiveZViewSpace / dirLight.m_effectiveShadowDistance);
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
	ANKI_LOOP while(cluster.m_pointLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = findLSB2(cluster.m_pointLightsMask);
		cluster.m_pointLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const PointLight light = u_pointLights2[idx];

		ANKI_BRANCH if(light.m_shadowAtlasTileScale >= 0.0)
		{
			const Vec3 frag2Light = light.m_position - worldPos;

			const ANKI_RP F32 shadowFactor =
				computeShadowFactorPointLight(light, frag2Light, u_shadowAtlasTex, u_linearAnyClampSampler);
			shadowFactors[min(maxShadowCastersPerFragment - 1u, shadowCasterCountPerFragment++)] = shadowFactor;
		}
	}

	// Spot lights
	ANKI_LOOP while(cluster.m_spotLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = findLSB2(cluster.m_spotLightsMask);
		cluster.m_spotLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const SpotLight light = u_spotLights[idx];

		ANKI_BRANCH if(light.m_shadowLayer != kMaxU32)
		{
			const ANKI_RP F32 shadowFactor =
				computeShadowFactorSpotLight(light, worldPos, u_shadowAtlasTex, u_linearAnyClampSampler);
			shadowFactors[min(maxShadowCastersPerFragment - 1u, shadowCasterCountPerFragment++)] = shadowFactor;
		}
	}

	// Store
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy),
			   Vec4(shadowFactors[0], shadowFactors[1], shadowFactors[2], shadowFactors[3]));
#else
	out_color = Vec4(shadowFactors[0], shadowFactors[1], shadowFactors[2], shadowFactors[3]);
#endif
}
