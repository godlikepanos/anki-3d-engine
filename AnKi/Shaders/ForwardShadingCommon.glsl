// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>
#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/Include/ModelTypes.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BINDLESS_SET(kMaterialSetBindless)

//
// Vert
//
#if defined(ANKI_VERTEX_SHADER)
layout(location = kVertexAttributeIdPosition) in Vec3 in_position;
#endif

//
// Frag
//
#if defined(ANKI_FRAGMENT_SHADER)
// Global resources
layout(set = kMaterialSetGlobal, binding = kMaterialBindingLinearClampSampler) uniform sampler u_linearAnyClampSampler;
layout(set = kMaterialSetGlobal, binding = kMaterialBindingDepthRt) uniform texture2D u_gbufferDepthRt;
layout(set = kMaterialSetGlobal, binding = kMaterialBindingLightVolume) uniform ANKI_RP texture3D u_lightVol;
#	define CLUSTERED_SHADING_SET kMaterialSetGlobal
#	define CLUSTERED_SHADING_UNIFORMS_BINDING kMaterialBindingClusterShadingUniforms
#	define CLUSTERED_SHADING_LIGHTS_BINDING kMaterialBindingClusterShadingLights
#	define CLUSTERED_SHADING_CLUSTERS_BINDING kMaterialBindingClusters
#	include <AnKi/Shaders/ClusteredShadingCommon.glsl>

layout(location = 0) out Vec4 out_color;

void packGBuffer(Vec4 color)
{
	out_color = Vec4(color.rgb, color.a);
}

ANKI_RP Vec4 readAnimatedTextureRgba(ANKI_RP texture2DArray tex, sampler sampl, F32 period, Vec2 uv, F32 time)
{
	const F32 layerCount = F32(textureSize(tex, 0).z);
	const F32 layer = mod(time * layerCount / period, layerCount);
	return texture(tex, sampl, Vec3(uv, layer));
}

// Iterate the clusters to compute the light color
Vec3 computeLightColorHigh(Vec3 diffCol, Vec3 worldPos)
{
	diffCol = diffuseLobe(diffCol);
	Vec3 outColor = Vec3(0.0);

	// Find the cluster and then the light counts
	Cluster cluster = getClusterFragCoord(gl_FragCoord.xyz);

	// Point lights
	ANKI_LOOP while(cluster.m_pointLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = findLSB2(cluster.m_pointLightsMask);
		cluster.m_pointLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const PointLight light = u_pointLights2[idx];

		const Vec3 diffC = diffCol * light.m_diffuseColor;

		const Vec3 frag2Light = light.m_position - worldPos;
		const F32 att = computeAttenuationFactor(light.m_squareRadiusOverOne, frag2Light);

#	if LOD > 1
		const F32 shadow = 1.0;
#	else
		F32 shadow = 1.0;
		if(light.m_shadowAtlasTileScale >= 0.0)
		{
			shadow = computeShadowFactorPointLight(light, frag2Light, u_shadowAtlasTex, u_linearAnyClampSampler);
		}
#	endif

		outColor += diffC * (att * shadow);
	}

	// Spot lights
	ANKI_LOOP while(cluster.m_spotLightsMask != ExtendedClusterObjectMask(0))
	{
		const I32 idx = findLSB2(cluster.m_spotLightsMask);
		cluster.m_spotLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const SpotLight light = u_spotLights[idx];

		const Vec3 diffC = diffCol * light.m_diffuseColor;

		const Vec3 frag2Light = light.m_position - worldPos;
		const F32 att = computeAttenuationFactor(light.m_squareRadiusOverOne, frag2Light);

		const Vec3 l = normalize(frag2Light);

		const F32 spot = computeSpotFactor(l, light.m_outerCos, light.m_innerCos, light.m_direction);

#	if LOD > 1
		const F32 shadow = 1.0;
#	else
		F32 shadow = 1.0;
		ANKI_BRANCH if(light.m_shadowLayer != kMaxU32)
		{
			shadow = computeShadowFactorSpotLight(light, worldPos, u_shadowAtlasTex, u_linearAnyClampSampler);
		}
#	endif

		outColor += diffC * (att * spot * shadow);
	}

	return outColor;
}

// Just read the light color from the vol texture
ANKI_RP Vec3 computeLightColorLow(ANKI_RP Vec3 diffCol, ANKI_RP Vec3 worldPos)
{
	const Vec2 uv = gl_FragCoord.xy / u_clusteredShading.m_renderingSize;
	const F32 linearDepth = linearizeDepth(gl_FragCoord.z, u_clusteredShading.m_near, u_clusteredShading.m_far);
	const Vec3 uvw =
		Vec3(uv, linearDepth
					 * (F32(u_clusteredShading.m_zSplitCount) / F32(u_clusteredShading.m_lightVolumeLastZSplit + 1u)));

	const ANKI_RP Vec3 light = textureLod(u_lightVol, u_linearAnyClampSampler, uvw, 0.0).rgb;
	return diffuseLobe(diffCol) * light;
}

void particleAlpha(ANKI_RP Vec4 color, ANKI_RP Vec4 scaleColor, ANKI_RP Vec4 biasColor)
{
	packGBuffer(color * scaleColor + biasColor);
}

void fog(ANKI_RP Vec3 color, ANKI_RP F32 fogAlphaScale, ANKI_RP F32 fogDistanceOfMaxThikness, F32 zVSpace)
{
	const Vec2 screenSize = 1.0 / u_clusteredShading.m_renderingSize;

	const Vec2 texCoords = gl_FragCoord.xy * screenSize;
	const F32 depth = textureLod(u_gbufferDepthRt, u_linearAnyClampSampler, texCoords, 0.0).r;
	F32 zFeatherFactor;

	const Vec4 fragPosVspace4 =
		u_clusteredShading.m_matrices.m_invertedProjectionJitter * Vec4(Vec3(UV_TO_NDC(texCoords), depth), 1.0);
	const F32 sceneZVspace = fragPosVspace4.z / fragPosVspace4.w;

	const F32 diff = max(0.0, zVSpace - sceneZVspace);

	zFeatherFactor = min(1.0, diff / fogDistanceOfMaxThikness);

	packGBuffer(Vec4(color, zFeatherFactor * fogAlphaScale));
}

#endif // defined(ANKI_FRAGMENT_SHADER)
