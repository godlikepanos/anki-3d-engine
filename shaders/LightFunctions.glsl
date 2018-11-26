// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#pragma once

#include <shaders/Functions.glsl>
#include <shaders/Pack.glsl>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

const U32 SHADOW_SAMPLE_COUNT = 16;
#if !defined(ESM_CONSTANT)
const F32 ESM_CONSTANT = 40.0;
#endif

// Fresnel term unreal
Vec3 F_Unreal(Vec3 specular, F32 VoH)
{
	return specular + (1.0 - specular) * pow(2.0, (-5.55473 * VoH - 6.98316) * VoH);
}

// Fresnel Schlick: "An Inexpensive BRDF Model for Physically-Based Rendering"
// It has lower VGRPs than F_Unreal
Vec3 F_Schlick(Vec3 specular, F32 VoH)
{
	F32 a = 1.0 - VoH;
	F32 a2 = a * a;
	F32 a5 = a2 * a2 * a; // a5 = a^5
	return /*saturate(50.0 * specular.g) */ a5 + (1.0 - a5) * specular;
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
F32 D_GGX(F32 roughness, F32 NoH)
{
	F32 a = roughness * roughness;
	F32 a2 = a * a;

	F32 D = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * D * D);
}

// Visibility term: Geometric shadowing divided by BRDF denominator
F32 V_Schlick(F32 roughness, F32 NoV, F32 NoL)
{
	F32 k = (roughness * roughness) * 0.5;
	F32 Vis_SchlickV = NoV * (1.0 - k) + k;
	F32 Vis_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

Vec3 envBRDF(Vec3 specular, F32 roughness, sampler2D integrationLut, F32 NoV)
{
	F32 a = roughness * roughness;
	F32 a2 = a * a;
	Vec2 envBRDF = textureLod(integrationLut, Vec2(a2, NoV), 0.0).xy;
	return specular * envBRDF.x + /*min(1.0, 50.0 * specular.g) */ envBRDF.y;
}

Vec3 diffuseLambert(Vec3 diffuse)
{
	return diffuse * (1.0 / PI);
}

// Performs BRDF specular lighting
Vec3 computeSpecularColorBrdf(GbufferInfo gbuffer, Vec3 viewDir, Vec3 frag2Light)
{
	Vec3 H = normalize(frag2Light + viewDir);

	F32 NoL = max(EPSILON, dot(gbuffer.m_normal, frag2Light));
	F32 VoH = max(EPSILON, dot(viewDir, H));
	F32 NoH = max(EPSILON, dot(gbuffer.m_normal, H));
	F32 NoV = max(EPSILON, dot(gbuffer.m_normal, viewDir));

	// F
#if 0
	Vec3 F = F_Unreal(gbuffer.m_specular, VoH);
#else
	Vec3 F = F_Schlick(gbuffer.m_specular, VoH);
#endif

	// D
	F32 D = D_GGX(gbuffer.m_roughness, NoH);

	// Vis
	F32 V = V_Schlick(gbuffer.m_roughness, NoV, NoL);

	return F * (V * D);
}

F32 computeSpotFactor(Vec3 l, F32 outerCos, F32 innerCos, Vec3 spotDir)
{
	F32 costheta = -dot(l, spotDir);
	F32 spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

U32 computeShadowSampleCount(const U32 COUNT, F32 zVSpace)
{
	const F32 MAX_DISTANCE = 5.0;

	F32 z = max(zVSpace, -MAX_DISTANCE);
	F32 sampleCountf = F32(COUNT) + z * (F32(COUNT) / MAX_DISTANCE);
	sampleCountf = max(sampleCountf, 1.0);
	U32 sampleCount = U32(sampleCountf);

	return sampleCount;
}

F32 computeShadowFactorSpot(Mat4 lightProjectionMat, Vec3 worldPos, F32 distance, sampler2D spotMapArr)
{
	Vec4 texCoords4 = lightProjectionMat * Vec4(worldPos, 1.0);
	Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const F32 near = LIGHT_FRUSTUM_NEAR_PLANE;
	const F32 far = distance;

	F32 linearDepth = linearizeDepth(texCoords3.z, near, far);

	F32 shadowFactor = textureLod(spotMapArr, texCoords3.xy, 0.0).r;

	return saturate(exp(ESM_CONSTANT * (shadowFactor - linearDepth)));
}

F32 computeShadowFactorOmni(Vec3 frag2Light, F32 radius, UVec2 atlasTiles, F32 tileSize, sampler2D shadowMap)
{
	Vec3 dir = -frag2Light;
	Vec3 dirabs = abs(dir);
	F32 dist = max(dirabs.x, max(dirabs.y, dirabs.z));

	const F32 near = LIGHT_FRUSTUM_NEAR_PLANE;
	const F32 far = radius;

	F32 linearDepth = (dist - near) / (far - near);

	// Read tex
	F32 shadowFactor;
	{
		// Convert cube coords
		U32 faceIdxu;
		Vec2 uv = convertCubeUvsu(dir, faceIdxu);

		// Clamp uv to a small value to avoid reading from other tiles due to bilinear filtering. It's not a perfect
		// solution but it works
		uv = clamp(uv, Vec2(0.001), Vec2(1.0 - 0.001));

		// Compute atlas tile
		atlasTiles >>= UVec2(faceIdxu * 5u);
		atlasTiles &= UVec2(31u);

		// Compute UV
		uv = (uv + Vec2(atlasTiles)) * tileSize;

		// Sample
		shadowFactor = textureLod(shadowMap, uv, 0.0).r;
	}

	return saturate(exp(ESM_CONSTANT * (shadowFactor - linearDepth)));
}

// Compute the cubemap texture lookup vector given the reflection vector (r) the radius squared of the probe (R2) and
// the frag pos in sphere space (f)
Vec3 computeCubemapVecAccurate(in Vec3 r, in F32 R2, in Vec3 f)
{
	// Compute the collision of the r to the inner part of the sphere
	// From now on we work on the sphere's space

	// Project the center of the sphere (it's zero now since we are in sphere space) in ray "f,r"
	Vec3 p = f - r * dot(f, r);

	// The collision to the sphere is point x where x = p + T * r
	// Because of the pythagorean theorem: R^2 = dot(p, p) + dot(T * r, T * r)
	// solving for T, T = R / |p|
	// then x becomes x = sqrt(R^2 - dot(p, p)) * r + p;
	F32 pp = dot(p, p);
	pp = min(pp, R2);
	F32 sq = sqrt(R2 - pp);
	Vec3 x = p + sq * r;

	return x;
}

// Cheap version of computeCubemapVecAccurate
Vec3 computeCubemapVecCheap(in Vec3 r, in F32 R2, in Vec3 f)
{
	return r;
}

F32 computeAttenuationFactor(F32 lightRadius, Vec3 frag2Light)
{
	F32 fragLightDist = dot(frag2Light, frag2Light);
	F32 att = 1.0 - fragLightDist * lightRadius;
	att = max(0.0, att);
	return att * att;
}

// Given the probe properties trace a ray inside the probe and find the cube tex coordinates to sample
Vec3 intersectProbe(Vec3 fragPos, // Ray origin
	Vec3 rayDir, // Ray direction
	Vec3 probeAabbMin,
	Vec3 probeAabbMax,
	Vec3 probeOrigin // Cubemap origin
)
{
	// Compute the intersection point
	F32 intresectionDist = rayAabbIntersectionInside(fragPos, rayDir, probeAabbMin, probeAabbMax);
	Vec3 intersectionPoint = fragPos + intresectionDist * rayDir;

	// Compute the cubemap vector
	return intersectionPoint - probeOrigin;
}

// Compute a weight (factor) of fragPos against some probe's bounds. The weight will be zero when fragPos is close to
// AABB bounds and 1.0 at fadeDistance and less.
F32 computeProbeBlendWeight(Vec3 fragPos, // Doesn't need to be inside the AABB
	Vec3 probeAabbMin,
	Vec3 probeAabbMax,
	F32 fadeDistance)
{
	// Compute the min distance of fragPos from the edges of the AABB
	Vec3 distFromMin = fragPos - probeAabbMin;
	Vec3 distFromMax = probeAabbMax - fragPos;
	Vec3 minDistVec = min(distFromMin, distFromMax);
	F32 minDist = min(minDistVec.x, min(minDistVec.y, minDistVec.z));

	// Use saturate because minDist might be negative.
	return saturate(minDist / fadeDistance);
}
