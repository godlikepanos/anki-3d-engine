// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#pragma once

#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/CollisionFunctions.glsl>
#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

// Do some EVSM magic with depth
Vec2 evsmProcessDepth(F32 depth)
{
	depth = 2.0 * depth - 1.0;
	const F32 pos = exp(kEvsmPositiveConstant * depth);
	const F32 neg = -exp(kEvsmNegativeConstant * depth);
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
	const Vec2 depthScale = kEvsmBias * 0.01 * Vec2(kEvsmPositiveConstant, kEvsmNegativeConstant) * evsmOccluderDepths;
	const Vec2 minVariance = depthScale * depthScale;

#if !ANKI_EVSM4
	return chebyshevUpperBound(shadowMapMoments.xy, evsmOccluderDepths.x, minVariance.x, kEvsmLightBleedingReduction);
#else
	const F32 pos =
		chebyshevUpperBound(shadowMapMoments.xy, evsmOccluderDepths.x, minVariance.x, kEvsmLightBleedingReduction);
	const F32 neg =
		chebyshevUpperBound(shadowMapMoments.zw, evsmOccluderDepths.y, minVariance.y, kEvsmLightBleedingReduction);
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
ANKI_RP Vec3 F_Schlick(ANKI_RP Vec3 f0, ANKI_RP F32 VoH)
{
	const ANKI_RP F32 f = pow(1.0 - VoH, 5.0);
	return f + f0 * (1.0 - f);
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
ANKI_RP F32 D_GGX(ANKI_RP F32 roughness, ANKI_RP F32 NoH, ANKI_RP Vec3 h, ANKI_RP Vec3 worldNormal)
{
#if 0 && ANKI_PLATFORM_MOBILE
	const ANKI_RP Vec3 NxH = cross(worldNormal, h);
	const ANKI_RP F32 oneMinusNoHSquared = dot(NxH, NxH);
#else
	const ANKI_RP F32 oneMinusNoHSquared = 1.0 - NoH * NoH;
#endif

	const ANKI_RP F32 a = roughness * roughness;
	const ANKI_RP F32 v = NoH * a;
	const ANKI_RP F32 k = a / (oneMinusNoHSquared + v * v);
	const ANKI_RP F32 d = k * k * (1.0 / kPi);
	return saturateRp(d);
}

// Visibility term: Geometric shadowing divided by BRDF denominator
ANKI_RP F32 V_Schlick(ANKI_RP F32 roughness, ANKI_RP F32 NoV, ANKI_RP F32 NoL)
{
	const ANKI_RP F32 k = (roughness * roughness) * 0.5;
	const ANKI_RP F32 Vis_SchlickV = NoV * (1.0 - k) + k;
	const ANKI_RP F32 Vis_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

// Visibility term: Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
ANKI_RP F32 V_SmithGGXCorrelatedFast(ANKI_RP F32 roughness, ANKI_RP F32 NoV, ANKI_RP F32 NoL)
{
	const ANKI_RP F32 a = roughness * roughness;
	const ANKI_RP F32 v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, a);
	return saturateRp(v);
}

ANKI_RP F32 Fd_Lambert()
{
	return 1.0 / kPi;
}

ANKI_RP Vec3 diffuseLobe(ANKI_RP Vec3 diffuse)
{
	return diffuse * Fd_Lambert();
}

// Performs BRDF specular lighting
ANKI_RP Vec3 specularIsotropicLobe(GbufferInfo gbuffer, Vec3 viewDir, Vec3 frag2Light)
{
	const ANKI_RP Vec3 H = normalize(frag2Light + viewDir);

	const ANKI_RP F32 NoL = max(0.0, dot(gbuffer.m_normal, frag2Light));
	const ANKI_RP F32 VoH = max(0.0, dot(viewDir, H));
	const ANKI_RP F32 NoH = max(0.0, dot(gbuffer.m_normal, H));
	const ANKI_RP F32 NoV = max(0.05, dot(gbuffer.m_normal, viewDir));

	// F
	const ANKI_RP Vec3 F = F_Schlick(gbuffer.m_f0, VoH);

	// D
	const ANKI_RP F32 D = D_GGX(gbuffer.m_roughness, NoH, H, gbuffer.m_normal);

	// Vis
	const ANKI_RP F32 V = V_SmithGGXCorrelatedFast(gbuffer.m_roughness, NoV, NoL);

	return F * (V * D);
}

Vec3 specularDFG(Vec3 F0, F32 roughness, texture2D integrationLut, sampler integrationLutSampler, F32 NoV)
{
	const Vec2 envBRDF = textureLod(integrationLut, integrationLutSampler, Vec2(roughness, NoV), 0.0).xy;
	return mix(envBRDF.xxx, envBRDF.yyy, F0);
}

ANKI_RP F32 computeSpotFactor(ANKI_RP Vec3 l, ANKI_RP F32 outerCos, ANKI_RP F32 innerCos, ANKI_RP Vec3 spotDir)
{
	const ANKI_RP F32 costheta = -dot(l, spotDir);
	const ANKI_RP F32 spotFactor = smoothstep(outerCos, innerCos, costheta);
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

ANKI_RP F32 computeShadowFactorSpotLight(SpotLight light, Vec3 worldPos, texture2D spotMap, sampler spotMapSampler)
{
	const Vec4 texCoords4 = light.m_textureMatrix * Vec4(worldPos, 1.0);
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const Vec4 shadowMoments = textureLod(spotMap, spotMapSampler, texCoords3.xy, 0.0);

	return evsmComputeShadowFactor(texCoords3.z, shadowMoments);
}

// Compute the shadow factor of point (omni) lights.
ANKI_RP F32 computeShadowFactorPointLight(PointLight light, Vec3 frag2Light, texture2D shadowMap,
										  sampler shadowMapSampler)
{
	const Vec3 dir = -frag2Light;
	const Vec3 dirabs = abs(dir);
	const F32 dist = max(dirabs.x, max(dirabs.y, dirabs.z));

	// 1) Project the dist to light's proj mat
	//
	const F32 near = kClusterObjectFrustumNearPlane;
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
	const Vec2 atlasOffset = light.m_shadowAtlasTileOffsets[faceIdxu].xy;

	// Compute UV
	uv *= Vec2(light.m_shadowAtlasTileScale);
	uv += atlasOffset;

	// Sample
	const Vec4 shadowMoments = textureLod(shadowMap, shadowMapSampler, uv, 0.0);

	// 3) Compare
	//
	const ANKI_RP F32 shadowFactor = evsmComputeShadowFactor(z, shadowMoments);

	return shadowFactor;
}

// Compute the shadow factor of a directional light
ANKI_RP F32 computeShadowFactorDirLight(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, texture2D shadowMap,
										sampler shadowMapSampler)
{
#define ANKI_FAST_CASCADES_WORKAROUND 1 // Doesn't make sense but it's super fast

#if ANKI_FAST_CASCADES_WORKAROUND
	// Assumes kMaxShadowCascades is 4
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
ANKI_RP F32 computeShadowFactorDirLight(Mat4 lightProjectionMat, Vec3 worldPos, texture2D shadowMap,
										samplerShadow shadowMapSampler)
{
	const Vec4 texCoords4 = lightProjectionMat * Vec4(worldPos, 1.0);
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const ANKI_RP F32 shadowFactor = textureLod(shadowMap, shadowMapSampler, texCoords3, 0.0);
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

ANKI_RP F32 computeAttenuationFactor(ANKI_RP F32 squareRadiusOverOne, ANKI_RP Vec3 frag2Light)
{
	const ANKI_RP F32 fragLightDist = dot(frag2Light, frag2Light);
	ANKI_RP F32 att = 1.0 - fragLightDist * squareRadiusOverOne;
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
	const F32 intresectionDist = testRayAabbInside(fragPos, rayDir, probeAabbMin, probeAabbMax);
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
ANKI_RP Vec3 sampleAmbientDice(ANKI_RP Vec3 posx, ANKI_RP Vec3 negx, ANKI_RP Vec3 posy, ANKI_RP Vec3 negy,
							   ANKI_RP Vec3 posz, ANKI_RP Vec3 negz, ANKI_RP Vec3 normal)
{
	const ANKI_RP Vec3 axisWeights = abs(normal);
	const ANKI_RP Vec3 uv = NDC_TO_UV(normal);

	ANKI_RP Vec3 col = mix(negx, posx, uv.x) * axisWeights.x;
	col += mix(negy, posy, uv.y) * axisWeights.y;
	col += mix(negz, posz, uv.z) * axisWeights.z;

	// Divide by weight
	col /= axisWeights.x + axisWeights.y + axisWeights.z + kEpsilonRp;

	return col;
}

// Sample the irradiance term from the clipmap
ANKI_RP Vec3 sampleGlobalIllumination(const Vec3 worldPos, const Vec3 normal, const GlobalIlluminationProbe probe,
									  ANKI_RP texture3D textures[kMaxVisibleGlobalIlluminationProbes],
									  sampler linearAnyClampSampler)
{
	// Find the UVW
	Vec3 uvw = (worldPos - probe.m_aabbMin) / (probe.m_aabbMax - probe.m_aabbMin);

	// The U contains the 6 directions so divide
	uvw.x /= 6.0;

	// Calmp it to avoid direction leaking
	uvw.x = clamp(uvw.x, probe.m_halfTexelSizeU, (1.0 / 6.0) - probe.m_halfTexelSizeU);

	// Read the irradiance
	ANKI_RP Vec3 irradiancePerDir[6u];
	ANKI_UNROLL for(U32 dir = 0u; dir < 6u; ++dir)
	{
		// Point to the correct UV
		Vec3 shiftedUVw = uvw;
		shiftedUVw.x += (1.0 / 6.0) * F32(dir);

		irradiancePerDir[dir] =
			textureLod(textures[nonuniformEXT(probe.m_textureIndex)], linearAnyClampSampler, shiftedUVw, 0.0).rgb;
	}

	// Sample the irradiance
	const ANKI_RP Vec3 irradiance =
		sampleAmbientDice(irradiancePerDir[0], irradiancePerDir[1], irradiancePerDir[2], irradiancePerDir[3],
						  irradiancePerDir[4], irradiancePerDir[5], normal);

	return irradiance;
}

U32 computeShadowCascadeIndex(F32 distance, F32 p, F32 effectiveShadowDistance, U32 shadowCascadeCount)
{
	const F32 shadowCascadeCountf = F32(shadowCascadeCount);
	F32 idx = pow(distance / effectiveShadowDistance, 1.0f / p) * shadowCascadeCountf;
	idx = min(idx, shadowCascadeCountf - 1.0f);
	return U32(idx);
}

/// Bring the indices of the closest cascades and a factor to blend them. To visualize what's going on go to
/// https://www.desmos.com/calculator/dwlbq2j55i
UVec2 computeShadowCascadeIndex2(F32 distance, F32 p, F32 effectiveShadowDistance, U32 shadowCascadeCount,
								 out F32 blendFactor)
{
	const F32 shadowCascadeCountf = F32(shadowCascadeCount);
	const F32 idx = pow(distance / effectiveShadowDistance, 1.0f / p) * shadowCascadeCountf;

	const U32 cascadeA = min(U32(idx), shadowCascadeCount - 1u);
	const U32 cascadeB = min(cascadeA + 1u, shadowCascadeCount - 1u);

	blendFactor = pow(fract(idx), 24.0);

	return UVec2(cascadeA, cascadeB);
}

/// To play with it use https://www.shadertoy.com/view/sttSDf
/// http://jcgt.org/published/0007/04/01/paper.pdf by Eric Heitz
/// Input v: view direction
/// Input alphaX, alphaY: roughness parameters
/// Input u1, u2: uniform random numbers
/// Output: normal sampled with PDF D_Ve(nE) = G1(v) * max(0, dot(v, nE)) * D(nE) / v.z
Vec3 sampleGgxVndf(Vec3 v, F32 alphaX, F32 alphaY, F32 u1, F32 u2)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	const Vec3 vH = normalize(Vec3(alphaX * v.x, alphaY * v.y, v.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	const F32 lensq = vH.x * vH.x + vH.y * vH.y;
	const Vec3 tangent1 = (lensq > 0.0) ? Vec3(-vH.y, vH.x, 0) * inversesqrt(lensq) : Vec3(1.0, 0.0, 0.0);
	const Vec3 tangent2 = cross(vH, tangent1);

	// Section 4.2: parameterization of the projected area
	const F32 r = sqrt(u1);
	const F32 phi = 2.0 * kPi * u2;
	const F32 t1 = r * cos(phi);
	F32 t2 = r * sin(phi);
	const F32 s = 0.5 * (1.0 + vH.z);
	t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

	// Section 4.3: reprojection onto hemisphere
	const Vec3 nH = t1 * tangent1 + t2 * tangent2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * vH;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	const Vec3 nE = normalize(Vec3(alphaX * nH.x, alphaY * nH.y, max(0.0, nH.z)));

	return nE;
}

/// Calculate the reflection vector based on roughness.
Vec3 sampleReflectionVector(Vec3 viewDir, Vec3 normal, F32 roughness, Vec2 uniformRandom)
{
	const Mat3 tbn = rotationFromDirection(normal);
	const Mat3 tbnT = transpose(tbn);
	const Vec3 viewDirTbn = tbnT * viewDir;

	Vec3 sampledNormalTbn = sampleGgxVndf(viewDirTbn, roughness, roughness, uniformRandom.x, uniformRandom.y);
	const Bool perfectReflection = false; // For debugging
	if(perfectReflection)
	{
		sampledNormalTbn = Vec3(0.0, 0.0, 1.0);
	}

	const Vec3 reflectedDirTbn = reflect(-viewDirTbn, sampledNormalTbn);

	// Transform reflected_direction back to the initial space.
	return tbn * reflectedDirTbn;
}
