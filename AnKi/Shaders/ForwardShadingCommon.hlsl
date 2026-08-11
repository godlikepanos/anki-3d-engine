// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/LightFunctions.hlsl>
#include <AnKi/Shaders/ClusteredShadingFunctions.hlsl>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>

#define FORWARD_SHADING 1
#include <AnKi/Shaders/MaterialShadersCommon.hlsl>

//
// Frag
//
#if ANKI_PIXEL_SHADER
struct PixelOut
{
	Vec4 m_color : SV_TARGET0;
};

Vec4 readAnimatedTextureRgba(Texture2DArray<Vec4> tex, SamplerState sampl, F32 period, Vec2 uv, F32 time)
{
	Vec2 texSize;
	F32 layerCount;
	F32 mipCount;
	tex.GetDimensions(0, texSize.x, texSize.y, layerCount, mipCount);

	const F32 layer = fmod(time * layerCount / period, layerCount);
	return tex.Sample(sampl, Vec3(uv, layer));
}

// Iterate the clusters to compute the light color. Assume a fully lambertian surface
Vec3 computeLightColorHigh(Vec3 albedo, Vec3 worldPos, Vec4 svPosition)
{
	const Vec3 brdf = diffuseLobe(albedo);
	Vec3 outColor = Vec3(0.0, 0.0, 0.0);

	// Find the cluster and then the light counts
	Cluster cluster = getClusterFragCoord(g_clusters, g_globalRendererConstants.m_clusterer, svPosition.xyz);

	// Lights
	U32 idx = 0;
	[loop] while((idx = iterateLights(cluster)) != kMaxU32)
	{
		const GpuSceneLight light = g_lights[idx];
		ANKI_ASSERT(light.m_isPointLight != light.m_isSpotLight);

		const Vec3 frag2Light = light.m_position - worldPos;
		F32 att = computeAttenuationFactor<F32>(light.m_influenceRadius, light.m_sourceRadius, frag2Light);

		att *= (light.m_isSpotLight) ? computeSpotFactor<F32>(normalize(frag2Light), light.m_outerCos, light.m_innerCos, light.m_direction) : 1.0;

		F32 shadow = 1.0;
		if(light.m_shadow)
		{
			if(light.m_isSpotLight)
			{
				shadow = computeShadowFactorSpotLight<F32>(light, worldPos, g_shadowAtlasTex, g_trilinearClampShadowSampler);
			}
			else
			{
				shadow = computeShadowFactorPointLight<F32>(light, frag2Light, g_shadowAtlasTex, g_trilinearClampShadowSampler);
			}
		}

		outColor += brdf * light.m_luminousIntensity * kPreExposure * (att * shadow);
	}

	return outColor;
}

// Just read the light color from the vol texture. See VolumetricLightingAccumulation.ankiprog on what exactly the volume contains and how you use it
Vec3 computeLightColorLow(Vec3 diffCol, Vec3 worldPos, Vec4 svPosition)
{
	Vec3 uvw;
	uvw.xy = svPosition.xy / g_globalRendererConstants.m_renderingSize;
	uvw.z = computeVolumeWTexCoord(svPosition.z, g_globalRendererConstants.m_clusterer.m_lightVolumeWMagic.x,
								   g_globalRendererConstants.m_clusterer.m_lightVolumeWMagic.y);

	const Vec3 light = g_lightVol.SampleLevel(g_trilinearClampSampler, uvw, 0.0).rgb;
	return diffuseLobe(diffCol) * light;
}

void fog(Vec3 color, F32 fogAlphaScale, F32 fogDistanceOfMaxThikness, F32 zVSpace, Vec2 svPosition, out PixelOut output)
{
	const Vec2 screenSize = 1.0 / g_globalRendererConstants.m_renderingSize;

	const Vec2 texCoords = svPosition * screenSize;
	const F32 depth = g_gbufferDepthTex.Sample(g_trilinearClampSampler, texCoords, 0.0).r;
	F32 zFeatherFactor;

	const Vec4 fragPosVspace4 = mul(g_globalRendererConstants.m_matrices.m_invertedProjectionJitter, Vec4(Vec3(uvToNdc(texCoords), depth), 1.0));
	const F32 sceneZVspace = fragPosVspace4.z / fragPosVspace4.w;

	const F32 diff = max(0.0, zVSpace - sceneZVspace);

	zFeatherFactor = min(1.0, diff / fogDistanceOfMaxThikness);

	output.m_color = Vec4(color, zFeatherFactor * fogAlphaScale);
}

#endif // ANKI_PIXEL_SHADER
