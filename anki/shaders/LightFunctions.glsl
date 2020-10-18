// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#pragma once

#include <anki/shaders/Functions.glsl>
#include <anki/shaders/Pack.glsl>
#include <anki/shaders/include/ClusteredShadingTypes.h>
#include <anki/shaders/include/ClusteredShadingFunctions.h>
#include <anki/shaders/include/Evsm.h>

// Do some EVSM magic with depth
Vec2 evsmProcessDepth(F32 depth)
{
	depth = 2.0 * depth - 1.0;
	const F32 pos = exp(EVSM_POSITIVE_CONSTANT * depth);
	const F32 neg = -exp(EVSM_NEGATIVE_CONSTANT * depth);
	return Vec2(pos, neg);
}

F32 linstep(F32 a, F32 b, F32 v)
{
	return saturate((v - a) / (b - a));
}

// Reduces VSM light bleedning
F32 reduceLightBleeding(F32 pMax, F32 amount)
{
	// Remove the [0, amount] tail and linearly rescale (amount, 1].
	return linstep(amount, 1.0, pMax);
}

F32 chebyshevUpperBound(Vec2 moments, F32 mean, F32 minVariance, F32 lightBleedingReduction)
{
	// Compute variance
	F32 variance = moments.y - (moments.x * moments.x);
	variance = max(variance, minVariance);

	// Compute probabilistic upper bound
	const F32 d = mean - moments.x;
	F32 pMax = variance / (variance + (d * d));

	pMax = reduceLightBleeding(pMax, lightBleedingReduction);

	// One-tailed Chebyshev
	return (mean <= moments.x) ? 1.0 : pMax;
}

// Compute the shadow factor of EVSM given the 2 depths
F32 evsmComputeShadowFactor(F32 occluderDepth, Vec4 shadowMapMoments)
{
	const Vec2 evsmOccluderDepths = evsmProcessDepth(occluderDepth);
	const Vec2 depthScale =
		EVSM_BIAS * 0.01 * Vec2(EVSM_POSITIVE_CONSTANT, EVSM_NEGATIVE_CONSTANT) * evsmOccluderDepths;
	const Vec2 minVariance = depthScale * depthScale;

#if !ANKI_EVSM4
	return chebyshevUpperBound(shadowMapMoments.xy, evsmOccluderDepths.x, minVariance.x, EVSM_LIGHT_BLEEDING_REDUCTION);
#else
	const F32 pos =
		chebyshevUpperBound(shadowMapMoments.xy, evsmOccluderDepths.x, minVariance.x, EVSM_LIGHT_BLEEDING_REDUCTION);
	const F32 neg =
		chebyshevUpperBound(shadowMapMoments.zw, evsmOccluderDepths.y, minVariance.y, EVSM_LIGHT_BLEEDING_REDUCTION);
	return min(pos, neg);
#endif
}

// Fresnel term unreal
// specular: The specular color aka F0
Vec3 F_Unreal(Vec3 specular, F32 VoH)
{
	return specular + (1.0 - specular) * pow(2.0, (-5.55473 * VoH - 6.98316) * VoH);
}

// Fresnel Schlick: "An Inexpensive BRDF Model for Physically-Based Rendering"
// It has lower VGRPs than F_Unreal
// specular: The specular color aka F0
Vec3 F_Schlick(Vec3 specular, F32 VoH)
{
	const F32 a = 1.0 - VoH;
	const F32 a2 = a * a;
	const F32 a5 = a2 * a2 * a; // a5 = a^5
	return /*saturate(50.0 * specular.g) */ a5 + (1.0 - a5) * specular;
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
F32 D_GGX(F32 roughness, F32 NoH)
{
	const F32 a = roughness * roughness;
	const F32 a2 = a * a;

	const F32 D = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * D * D);
}

// Visibility term: Geometric shadowing divided by BRDF denominator
F32 V_Schlick(F32 roughness, F32 NoV, F32 NoL)
{
	const F32 k = (roughness * roughness) * 0.5;
	const F32 Vis_SchlickV = NoV * (1.0 - k) + k;
	const F32 Vis_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

Vec3 envBRDF(Vec3 specular, F32 roughness, texture2D integrationLut, sampler integrationLutSampler, F32 NoV)
{
	const Vec2 envBRDF = textureLod(integrationLut, integrationLutSampler, Vec2(roughness, NoV), 0.0).xy;
	return specular * envBRDF.x + min(1.0, 50.0 * specular.g) * envBRDF.y;
}

Vec3 diffuseLambert(Vec3 diffuse)
{
	return diffuse * (1.0 / PI);
}

// Performs BRDF specular lighting
Vec3 computeSpecularColorBrdf(GbufferInfo gbuffer, Vec3 viewDir, Vec3 frag2Light)
{
	const Vec3 H = normalize(frag2Light + viewDir);

	const F32 NoL = max(EPSILON, dot(gbuffer.m_normal, frag2Light));
	const F32 VoH = max(EPSILON, dot(viewDir, H));
	const F32 NoH = max(EPSILON, dot(gbuffer.m_normal, H));
	const F32 NoV = max(EPSILON, dot(gbuffer.m_normal, viewDir));

	// F
#if 0
	const Vec3 F = F_Unreal(gbuffer.m_specular, VoH);
#else
	const Vec3 F = F_Schlick(gbuffer.m_specular, VoH);
#endif

	// D
	const F32 D = D_GGX(gbuffer.m_roughness, NoH);

	// Vis
	const F32 V = V_Schlick(gbuffer.m_roughness, NoV, NoL);

	return F * (V * D);
}

F32 computeSpotFactor(Vec3 l, F32 outerCos, F32 innerCos, Vec3 spotDir)
{
	const F32 costheta = -dot(l, spotDir);
	const F32 spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

U32 computeShadowSampleCount(const U32 COUNT, F32 zVSpace)
{
	const F32 MAX_DISTANCE = 5.0;

	const F32 z = max(zVSpace, -MAX_DISTANCE);
	F32 sampleCountf = F32(COUNT) + z * (F32(COUNT) / MAX_DISTANCE);
	sampleCountf = max(sampleCountf, 1.0);
	const U32 sampleCount = U32(sampleCountf);

	return sampleCount;
}

F32 computeShadowFactorSpotLight(SpotLight light, Vec3 worldPos, texture2D spotMap, sampler spotMapSampler)
{
	const Vec4 texCoords4 = light.m_texProjectionMat * Vec4(worldPos, 1.0);
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const Vec4 shadowMoments = textureLod(spotMap, spotMapSampler, texCoords3.xy, 0.0);

	return evsmComputeShadowFactor(texCoords3.z, shadowMoments);
}

// Compute the shadow factor of point (omni) lights.
F32 computeShadowFactorPointLight(PointLight light, Vec3 frag2Light, texture2D shadowMap, sampler shadowMapSampler)
{
	const Vec3 dir = -frag2Light;
	const Vec3 dirabs = abs(dir);
	const F32 dist = max(dirabs.x, max(dirabs.y, dirabs.z));

	// 1) Project the dist to light's proj mat
	//
	const F32 near = LIGHT_FRUSTUM_NEAR_PLANE;
	const F32 far = light.m_radius;
	const F32 g = near - far;

	const F32 zVSpace = -dist;
	const F32 w = -zVSpace;
	F32 z = (far * zVSpace + far * near) / g;
	z /= w;

	// 2) Read shadow tex
	//

	// Convert cube coords
	U32 faceIdxu;
	Vec2 uv = convertCubeUvsu(dir, faceIdxu);

	// Get the atlas offset
	Vec2 atlasOffset;
	atlasOffset.x = light.m_shadowAtlasTileOffsets[faceIdxu >> 1u][(faceIdxu & 1u) << 1u];
	atlasOffset.y = light.m_shadowAtlasTileOffsets[faceIdxu >> 1u][((faceIdxu & 1u) << 1u) + 1u];

	// Compute UV
	uv = fma(uv, Vec2(light.m_shadowAtlasTileScale), atlasOffset);

	// Sample
	const Vec4 shadowMoments = textureLod(shadowMap, shadowMapSampler, uv, 0.0);

	// 3) Compare
	//
	const F32 shadowFactor = evsmComputeShadowFactor(z, shadowMoments);

	return shadowFactor;
}

// Compute the shadow factor of a directional light
F32 computeShadowFactorDirLight(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, texture2D shadowMap,
								sampler shadowMapSampler)
{
#define ANKI_FAST_CASCADES_WORKAROUND 1 // Doesn't make sense but it's super fast

#if ANKI_FAST_CASCADES_WORKAROUND
	// Assumes MAX_SHADOW_CASCADES is 4
	Mat4 lightProjectionMat;
	switch(cascadeIdx)
	{
	case 0:
		lightProjectionMat = light.m_textureMatrices[0];
		break;
	case 1:
		lightProjectionMat = light.m_textureMatrices[1];
		break;
	case 2:
		lightProjectionMat = light.m_textureMatrices[2];
		break;
	default:
		lightProjectionMat = light.m_textureMatrices[3];
	}
#else
	const Mat4 lightProjectionMat = light.m_textureMatrices[cascadeIdx];
#endif

	const Vec4 texCoords4 = lightProjectionMat * Vec4(worldPos, 1.0);
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const Vec4 shadowMoments = textureLod(shadowMap, shadowMapSampler, texCoords3.xy, 0.0);

	return evsmComputeShadowFactor(texCoords3.z, shadowMoments);
}

// Compute the shadow factor of a directional light
F32 computeShadowFactorDirLight(Mat4 lightProjectionMat, Vec3 worldPos, texture2D shadowMap,
								samplerShadow shadowMapSampler)
{
	const Vec4 texCoords4 = lightProjectionMat * Vec4(worldPos, 1.0);
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const F32 shadowFactor = textureLod(shadowMap, shadowMapSampler, texCoords3, 0.0);
	return shadowFactor;
}

// Compute the cubemap texture lookup vector given the reflection vector (r) the radius squared of the probe (R2) and
// the frag pos in sphere space (f)
Vec3 computeCubemapVecAccurate(Vec3 r, F32 R2, Vec3 f)
{
	// Compute the collision of the r to the inner part of the sphere
	// From now on we work on the sphere's space

	// Project the center of the sphere (it's zero now since we are in sphere space) in ray "f,r"
	const Vec3 p = f - r * dot(f, r);

	// The collision to the sphere is point x where x = p + T * r
	// Because of the pythagorean theorem: R^2 = dot(p, p) + dot(T * r, T * r)
	// solving for T, T = R / |p|
	// then x becomes x = sqrt(R^2 - dot(p, p)) * r + p;
	F32 pp = dot(p, p);
	pp = min(pp, R2);
	const F32 sq = sqrt(R2 - pp);
	const Vec3 x = p + sq * r;

	return x;
}

// Cheap version of computeCubemapVecAccurate
Vec3 computeCubemapVecCheap(Vec3 r, F32 R2, Vec3 f)
{
	return r;
}

F32 computeAttenuationFactor(F32 squareRadiusOverOne, Vec3 frag2Light)
{
	const F32 fragLightDist = dot(frag2Light, frag2Light);
	F32 att = 1.0 - fragLightDist * squareRadiusOverOne;
	att = max(0.0, att);
	return att * att;
}

// Given the probe properties trace a ray inside the probe and find the cube tex coordinates to sample
Vec3 intersectProbe(Vec3 fragPos, // Ray origin
					Vec3 rayDir, // Ray direction
					Vec3 probeAabbMin, Vec3 probeAabbMax,
					Vec3 probeOrigin // Cubemap origin
)
{
	// Compute the intersection point
	const F32 intresectionDist = rayAabbIntersectionInside(fragPos, rayDir, probeAabbMin, probeAabbMax);
	const Vec3 intersectionPoint = fragPos + intresectionDist * rayDir;

	// Compute the cubemap vector
	return intersectionPoint - probeOrigin;
}

// Compute a weight (factor) of fragPos against some probe's bounds. The weight will be zero when fragPos is close to
// AABB bounds and 1.0 at fadeDistance and less.
F32 computeProbeBlendWeight(Vec3 fragPos, // Doesn't need to be inside the AABB
							Vec3 probeAabbMin, Vec3 probeAabbMax, F32 fadeDistance)
{
	// Compute the min distance of fragPos from the edges of the AABB
	const Vec3 distFromMin = fragPos - probeAabbMin;
	const Vec3 distFromMax = probeAabbMax - fragPos;
	const Vec3 minDistVec = min(distFromMin, distFromMax);
	const F32 minDist = min(minDistVec.x, min(minDistVec.y, minDistVec.z));

	// Use saturate because minDist might be negative.
	return saturate(minDist / fadeDistance);
}

// Given the value of the 6 faces of the dice and a normal, sample the correct weighted value.
// https://www.shadertoy.com/view/XtcBDB
Vec3 sampleAmbientDice(Vec3 posx, Vec3 negx, Vec3 posy, Vec3 negy, Vec3 posz, Vec3 negz, Vec3 normal)
{
	const Vec3 axisWeights = abs(normal);
	const Vec3 uv = NDC_TO_UV(normal);

	Vec3 col = mix(negx, posx, uv.x) * axisWeights.x;
	col += mix(negy, posy, uv.y) * axisWeights.y;
	col += mix(negz, posz, uv.z) * axisWeights.z;

	// Divide by weight
	col /= axisWeights.x + axisWeights.y + axisWeights.z + EPSILON;

	return col;
}

// Sample the irradiance term from the clipmap
Vec3 sampleGlobalIllumination(const Vec3 worldPos, const Vec3 normal, const GlobalIlluminationProbe probe,
							  texture3D textures[MAX_VISIBLE_GLOBAL_ILLUMINATION_PROBES], sampler linearAnyClampSampler)
{
	// Find the UVW
	Vec3 uvw = (worldPos - probe.m_aabbMin) / (probe.m_aabbMax - probe.m_aabbMin);

	// The U contains the 6 directions so divide
	uvw.x /= 6.0;

	// Calmp it to avoid direction leaking
	uvw.x = clamp(uvw.x, probe.m_halfTexelSizeU, (1.0 / 6.0) - probe.m_halfTexelSizeU);

	// Read the irradiance
	Vec3 irradiancePerDir[6u];
	ANKI_UNROLL for(U32 dir = 0u; dir < 6u; ++dir)
	{
		// Point to the correct UV
		Vec3 shiftedUVw = uvw;
		shiftedUVw.x += (1.0 / 6.0) * F32(dir);

		irradiancePerDir[dir] =
			textureLod(textures[nonuniformEXT(probe.m_textureIndex)], linearAnyClampSampler, shiftedUVw, 0.0).rgb;
	}

	// Sample the irradiance
	const Vec3 irradiance = sampleAmbientDice(irradiancePerDir[0], irradiancePerDir[1], irradiancePerDir[2],
											  irradiancePerDir[3], irradiancePerDir[4], irradiancePerDir[5], normal);

	return irradiance;
}
