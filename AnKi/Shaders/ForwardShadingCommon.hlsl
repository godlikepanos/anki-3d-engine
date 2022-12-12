// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BINDLESS_SET(kMaterialSetBindless)

//
// Frag
//
#if defined(ANKI_FRAGMENT_SHADER)
// Global resources
[[vk::binding(kMaterialBindingLinearClampSampler, kMaterialSetGlobal)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(kMaterialBindingDepthRt, kMaterialSetGlobal)]] Texture2D g_gbufferDepthTex;
[[vk::binding(kMaterialBindingLightVolume, kMaterialSetGlobal)]] Texture3D<RVec4> g_lightVol;
[[vk::binding(kMaterialBindingShadowSampler, kMaterialSetGlobal)]] SamplerComparisonState g_shadowSampler;
#	define CLUSTERED_SHADING_SET kMaterialSetGlobal
#	define CLUSTERED_SHADING_UNIFORMS_BINDING kMaterialBindingClusterShadingUniforms
#	define CLUSTERED_SHADING_LIGHTS_BINDING kMaterialBindingClusterShadingLights
#	define CLUSTERED_SHADING_CLUSTERS_BINDING kMaterialBindingClusters
#	include <AnKi/Shaders/ClusteredShadingCommon.hlsl>

struct FragOut
{
	RVec4 m_color : SV_TARGET0;
};

void packGBuffer(Vec4 color, out FragOut output)
{
	output.m_color = RVec4(color.rgb, color.a);
}

RVec4 readAnimatedTextureRgba(Texture2DArray<RVec4> tex, SamplerState sampl, F32 period, Vec2 uv, F32 time)
{
	Vec2 texSize;
	F32 layerCount;
	F32 mipCount;
	tex.GetDimensions(0, texSize.x, texSize.y, layerCount, mipCount);

	const F32 layer = fmod(time * layerCount / period, layerCount);
	return tex.Sample(sampl, Vec3(uv, layer));
}

// Iterate the clusters to compute the light color
Vec3 computeLightColorHigh(Vec3 diffCol, Vec3 worldPos, Vec4 svPosition)
{
	diffCol = diffuseLobe(diffCol);
	Vec3 outColor = Vec3(0.0, 0.0, 0.0);

	// Find the cluster and then the light counts
	Cluster cluster = getClusterFragCoord(svPosition.xyz);

	// Point lights
	[loop] while(cluster.m_pointLightsMask != 0)
	{
		const I32 idx = firstbitlow2(cluster.m_pointLightsMask);
		cluster.m_pointLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const PointLight light = g_pointLights[idx];

		const Vec3 diffC = diffCol * light.m_diffuseColor;

		const Vec3 frag2Light = light.m_position - worldPos;
		const F32 att = computeAttenuationFactor(light.m_squareRadiusOverOne, frag2Light);

#	if defined(ANKI_LOD) && ANKI_LOD > 1
		const F32 shadow = 1.0;
#	else
		F32 shadow = 1.0;
		if(light.m_shadowAtlasTileScale >= 0.0)
		{
			shadow = computeShadowFactorPointLight(light, frag2Light, g_shadowAtlasTex, g_shadowSampler);
		}
#	endif

		outColor += diffC * (att * shadow);
	}

	// Spot lights
	[loop] while(cluster.m_spotLightsMask != 0)
	{
		const I32 idx = firstbitlow2(cluster.m_spotLightsMask);
		cluster.m_spotLightsMask &= ~(ExtendedClusterObjectMask(1) << ExtendedClusterObjectMask(idx));
		const SpotLight light = g_spotLights[idx];

		const Vec3 diffC = diffCol * light.m_diffuseColor;

		const Vec3 frag2Light = light.m_position - worldPos;
		const F32 att = computeAttenuationFactor(light.m_squareRadiusOverOne, frag2Light);

		const Vec3 l = normalize(frag2Light);

		const F32 spot = computeSpotFactor(l, light.m_outerCos, light.m_innerCos, light.m_direction);

#	if defined(ANKI_LOD) && ANKI_LOD > 1
		const F32 shadow = 1.0;
#	else
		F32 shadow = 1.0;
		[branch] if(light.m_shadowLayer != kMaxU32)
		{
			shadow = computeShadowFactorSpotLight(light, worldPos, g_shadowAtlasTex, g_shadowSampler);
		}
#	endif

		outColor += diffC * (att * spot * shadow);
	}

	return outColor;
}

// Just read the light color from the vol texture
RVec3 computeLightColorLow(RVec3 diffCol, RVec3 worldPos, Vec4 svPosition)
{
	ANKI_MAYBE_UNUSED(worldPos);

	const Vec2 uv = svPosition.xy / g_clusteredShading.m_renderingSize;
	const F32 linearDepth = linearizeDepth(svPosition.z, g_clusteredShading.m_near, g_clusteredShading.m_far);
	const F32 w =
		linearDepth * (F32(g_clusteredShading.m_zSplitCount) / F32(g_clusteredShading.m_lightVolumeLastZSplit + 1u));
	const Vec3 uvw = Vec3(uv, w);

	const RVec3 light = g_lightVol.SampleLevel(g_linearAnyClampSampler, uvw, 0.0).rgb;
	return diffuseLobe(diffCol) * light;
}

void particleAlpha(RVec4 color, RVec4 scaleColor, RVec4 biasColor, out FragOut output)
{
	packGBuffer(color * scaleColor + biasColor, output);
}

void fog(RVec3 color, RF32 fogAlphaScale, RF32 fogDistanceOfMaxThikness, F32 zVSpace, Vec2 svPosition,
		 out FragOut output)
{
	const Vec2 screenSize = 1.0 / g_clusteredShading.m_renderingSize;

	const Vec2 texCoords = svPosition * screenSize;
	const F32 depth = g_gbufferDepthTex.Sample(g_linearAnyClampSampler, texCoords, 0.0).r;
	F32 zFeatherFactor;

	const Vec4 fragPosVspace4 =
		mul(g_clusteredShading.m_matrices.m_invertedProjectionJitter, Vec4(Vec3(uvToNdc(texCoords), depth), 1.0));
	const F32 sceneZVspace = fragPosVspace4.z / fragPosVspace4.w;

	const F32 diff = max(0.0, zVSpace - sceneZVspace);

	zFeatherFactor = min(1.0, diff / fogDistanceOfMaxThikness);

	packGBuffer(Vec4(color, zFeatherFactor * fogAlphaScale), output);
}

#endif // defined(ANKI_FRAGMENT_SHADER)
