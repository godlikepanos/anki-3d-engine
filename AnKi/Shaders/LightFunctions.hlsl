// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#pragma once

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/CollisionFunctions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

constexpr Vec2 kPoissonDisk[4u] = {Vec2(-0.94201624, -0.39906216), Vec2(0.94558609, -0.76890725),
								   Vec2(-0.094184101, -0.92938870), Vec2(0.34495938, 0.29387760)};
constexpr RF32 kPcfScale = 2.0f;

// Fresnel term unreal
// specular: The specular color aka F0
Vec3 F_Unreal(Vec3 specular, F32 VoH)
{
	return specular + (1.0 - specular) * pow(2.0, (-5.55473 * VoH - 6.98316) * VoH);
}

// Fresnel Schlick: "An Inexpensive BRDF Model for Physically-Based Rendering"
// It has lower VGRPs than F_Unreal
RVec3 F_Schlick(RVec3 f0, RF32 VoH)
{
	const RF32 f = pow(1.0 - VoH, 5.0);
	return f + f0 * (1.0 - f);
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
RF32 D_GGX(RF32 roughness, RF32 NoH, RVec3 h, RVec3 worldNormal)
{
#if 0 && ANKI_PLATFORM_MOBILE
	const RVec3 NxH = cross(worldNormal, h);
	const RF32 oneMinusNoHSquared = dot(NxH, NxH);
#else
	const RF32 oneMinusNoHSquared = 1.0 - NoH * NoH;
	ANKI_MAYBE_UNUSED(h);
	ANKI_MAYBE_UNUSED(worldNormal);
#endif

	const RF32 a = roughness * roughness;
	const RF32 v = NoH * a;
	const RF32 k = a / (oneMinusNoHSquared + v * v);
	const RF32 d = k * k * (1.0 / kPi);
	return saturate(d);
}

// Visibility term: Geometric shadowing divided by BRDF denominator
RF32 V_Schlick(RF32 roughness, RF32 NoV, RF32 NoL)
{
	const RF32 k = (roughness * roughness) * 0.5;
	const RF32 Vis_SchlickV = NoV * (1.0 - k) + k;
	const RF32 Vis_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

// Visibility term: Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
RF32 V_SmithGGXCorrelatedFast(RF32 roughness, RF32 NoV, RF32 NoL)
{
	const RF32 a = roughness * roughness;
	const RF32 v = 0.5 / lerp(2.0 * NoL * NoV, NoL + NoV, a);
	return saturate(v);
}

RF32 Fd_Lambert()
{
	return 1.0 / kPi;
}

RVec3 diffuseLobe(RVec3 diffuse)
{
	return diffuse * Fd_Lambert();
}

// Performs BRDF specular lighting
RVec3 specularIsotropicLobe(GbufferInfo gbuffer, Vec3 viewDir, Vec3 frag2Light)
{
	const RVec3 H = normalize(frag2Light + viewDir);

	const RF32 NoL = max(0.0, dot(gbuffer.m_normal, frag2Light));
	const RF32 VoH = max(0.0, dot(viewDir, H));
	const RF32 NoH = max(0.0, dot(gbuffer.m_normal, H));
	const RF32 NoV = max(0.05, dot(gbuffer.m_normal, viewDir));

	// F
	const RVec3 F = F_Schlick(gbuffer.m_f0, VoH);

	// D
	const RF32 D = D_GGX(gbuffer.m_roughness, NoH, H, gbuffer.m_normal);

	// Vis
	const RF32 V = V_SmithGGXCorrelatedFast(gbuffer.m_roughness, NoV, NoL);

	return F * (V * D);
}

Vec3 specularDFG(RVec3 F0, RF32 roughness, Texture2D<RVec4> integrationLut, SamplerState integrationLutSampler, F32 NoV)
{
	const Vec2 envBRDF = integrationLut.SampleLevel(integrationLutSampler, Vec2(roughness, NoV), 0.0).xy;
	return lerp(envBRDF.xxx, envBRDF.yyy, F0);
}

RF32 computeSpotFactor(RVec3 l, RF32 outerCos, RF32 innerCos, RVec3 spotDir)
{
	const RF32 costheta = -dot(l, spotDir);
	const RF32 spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

RF32 computeShadowFactorSpotLightPcf(SpotLight light, Vec3 worldPos, Texture2D shadowTex,
									 SamplerComparisonState shadowMapSampler, RF32 randFactor)
{
	const Vec4 texCoords4 = mul(light.m_textureMatrix, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	Vec2 texSize;
	F32 mipCount;
	shadowTex.GetDimensions(0, texSize.x, texSize.y, mipCount);
	const Vec2 smTexelSize = 1.0 / texSize;

	const F32 sinTheta = sin(randFactor * 2.0 * kPi);
	const F32 cosTheta = cos(randFactor * 2.0 * kPi);

	RF32 shadow = 0.0;
	[unroll] for(U32 i = 0u; i < 4u; ++i)
	{
		const Vec2 diskPoint = kPoissonDisk[i] * kPcfScale;

		// Rotate the disk point
		Vec2 rotatedDiskPoint;
		rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
		rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

		// Offset calculation
		const Vec2 newUv = texCoords3.xy + rotatedDiskPoint * smTexelSize;

		shadow += shadowTex.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
	}

	shadow /= 4.0;

	return shadow;
}

RF32 computeShadowFactorSpotLight(SpotLight light, Vec3 worldPos, Texture2D shadowTex,
								  SamplerComparisonState shadowMapSampler)
{
	const Vec4 texCoords4 = mul(light.m_textureMatrix, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;
	return shadowTex.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
}

// Compute the shadow factor of point (omni) lights.
RF32 computeShadowFactorPointLightGeneric(PointLight light, Vec3 frag2Light, Texture2D shadowMap,
										  SamplerComparisonState shadowMapSampler, RF32 randFactor, Bool pcf)
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
	uv *= Vec2(light.m_shadowAtlasTileScale, light.m_shadowAtlasTileScale);
	uv += atlasOffset;

	// Sample
	RF32 shadow;
	if(pcf)
	{
		F32 mipCount;
		Vec2 smTexelSize;
		shadowMap.GetDimensions(0, smTexelSize.x, smTexelSize.y, mipCount);
		smTexelSize = 1.0 / smTexelSize;

		const F32 sinTheta = sin(randFactor * 2.0 * kPi);
		const F32 cosTheta = cos(randFactor * 2.0 * kPi);

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < 4u; ++i)
		{
			const Vec2 diskPoint = kPoissonDisk[i] * kPcfScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = uv + rotatedDiskPoint * smTexelSize;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, z);
		}

		shadow /= 4.0;
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, uv, z);
	}

	return shadow;
}

RF32 computeShadowFactorPointLight(PointLight light, Vec3 frag2Light, Texture2D shadowMap,
								   SamplerComparisonState shadowMapSampler)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, -1.0, false);
}

RF32 computeShadowFactorPointLightPcf(PointLight light, Vec3 frag2Light, Texture2D shadowMap,
									  SamplerComparisonState shadowMapSampler, RF32 randFactor)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, randFactor, true);
}

// Compute the shadow factor of a directional light
RF32 computeShadowFactorDirLightGeneric(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
										SamplerComparisonState shadowMapSampler, RF32 randFactor, Bool pcf)
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

	const Vec4 texCoords4 = mul(lightProjectionMat, Vec4(worldPos, 1.0));
	Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	RF32 shadow;
	if(pcf)
	{
		F32 mipCount;
		Vec2 smTexelSize;
		shadowMap.GetDimensions(0, smTexelSize.x, smTexelSize.y, mipCount);
		smTexelSize = 1.0 / smTexelSize;

		const F32 sinTheta = sin(randFactor * 2.0 * kPi);
		const F32 cosTheta = cos(randFactor * 2.0 * kPi);

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < 4u; ++i)
		{
			const Vec2 diskPoint = kPoissonDisk[i] * kPcfScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			Vec2 newUv = texCoords3.xy + rotatedDiskPoint * smTexelSize;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
		}

		shadow /= 4.0;
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
	}

	return shadow;
}

RF32 computeShadowFactorDirLight(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
								 SamplerComparisonState shadowMapSampler)
{
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, -1.0, false);
}

RF32 computeShadowFactorDirLightPcf(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
									SamplerComparisonState shadowMapSampler, F32 randFactor)
{
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, randFactor,
											  true);
}

// Compute the shadow factor of a directional light
RF32 computeShadowFactorDirLight(Mat4 lightProjectionMat, Vec3 worldPos, Texture2D<RVec4> shadowMap,
								 SamplerComparisonState shadowMapSampler)
{
	const Vec4 texCoords4 = mul(lightProjectionMat, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const RF32 shadowFactor = shadowMap.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
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
	ANKI_MAYBE_UNUSED(R2);
	ANKI_MAYBE_UNUSED(f);
	return r;
}

RF32 computeAttenuationFactor(RF32 squareRadiusOverOne, RVec3 frag2Light)
{
	const RF32 fragLightDist = dot(frag2Light, frag2Light);
	RF32 att = 1.0 - fragLightDist * squareRadiusOverOne;
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
RVec3 sampleAmbientDice(RVec3 posx, RVec3 negx, RVec3 posy, RVec3 negy, RVec3 posz, RVec3 negz, RVec3 normal)
{
	const RVec3 axisWeights = abs(normal);
	const RVec3 uv = ndcToUv(normal);

	RVec3 col = lerp(negx, posx, uv.x) * axisWeights.x;
	col += lerp(negy, posy, uv.y) * axisWeights.y;
	col += lerp(negz, posz, uv.z) * axisWeights.z;

	// Divide by weight
	col /= axisWeights.x + axisWeights.y + axisWeights.z + kEpsilonRF32;

	return col;
}

// Sample the irradiance term from the clipmap
RVec3 sampleGlobalIllumination(const Vec3 worldPos, const Vec3 normal, const GlobalIlluminationProbe probe,
							   Texture3D<RVec4> textures[kMaxVisibleGlobalIlluminationProbes],
							   SamplerState linearAnyClampSampler)
{
	// Find the UVW
	Vec3 uvw = (worldPos - probe.m_aabbMin) / (probe.m_aabbMax - probe.m_aabbMin);

	// The U contains the 6 directions so divide
	uvw.x /= 6.0;

	// Calmp it to avoid direction leaking
	uvw.x = clamp(uvw.x, probe.m_halfTexelSizeU, (1.0 / 6.0) - probe.m_halfTexelSizeU);

	// Read the irradiance
	RVec3 irradiancePerDir[6u];
	[unroll] for(U32 dir = 0u; dir < 6u; ++dir)
	{
		// Point to the correct UV
		Vec3 shiftedUVw = uvw;
		shiftedUVw.x += (1.0 / 6.0) * F32(dir);

		irradiancePerDir[dir] = textures[NonUniformResourceIndex(probe.m_textureIndex)]
									.SampleLevel(linearAnyClampSampler, shiftedUVw, 0.0)
									.rgb;
	}

	// Sample the irradiance
	const RVec3 irradiance = sampleAmbientDice(irradiancePerDir[0], irradiancePerDir[1], irradiancePerDir[2],
											   irradiancePerDir[3], irradiancePerDir[4], irradiancePerDir[5], normal);

	return irradiance;
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
	const Vec3 tangent1 = (lensq > 0.0) ? Vec3(-vH.y, vH.x, 0) / sqrt(lensq) : Vec3(1.0, 0.0, 0.0);
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
	const Vec3 viewDirTbn = mul(tbnT, viewDir);

	Vec3 sampledNormalTbn = sampleGgxVndf(viewDirTbn, roughness, roughness, uniformRandom.x, uniformRandom.y);
	const Bool perfectReflection = false; // For debugging
	if(perfectReflection)
	{
		sampledNormalTbn = Vec3(0.0, 0.0, 1.0);
	}

	const Vec3 reflectedDirTbn = reflect(-viewDirTbn, sampledNormalTbn);

	// Transform reflected_direction back to the initial space.
	return mul(tbn, reflectedDirTbn);
}

/// Get the index of the cascade given the distance from zero.
U32 computeShadowCascadeIndex(F32 distance, Vec4 cascadeDistances, U32 shadowCascadeCount)
{
	U32 cascade;
	if(distance < cascadeDistances[0u])
	{
		cascade = 0u;
	}
	else if(distance < cascadeDistances[1u])
	{
		cascade = 1u;
	}
	else if(distance < cascadeDistances[2u])
	{
		cascade = 2u;
	}
	else
	{
		cascade = 3u;
	}

	return min(shadowCascadeCount - 1u, cascade);
}

/// Bring the indices of the closest cascades and a factor to blend them. To visualize what's going on go to:
/// https://www.desmos.com/calculator/g1ibye6ebg
UVec2 computeShadowCascadeIndex2(F32 distance, Vec4 cascadeDistances, U32 shadowCascadeCount, out RF32 factor)
{
	const U32 cascade = computeShadowCascadeIndex(distance, cascadeDistances, shadowCascadeCount);
	const U32 nextCascade = min(cascade + 1u, shadowCascadeCount - 1u);

	const F32 minDist = (cascade == 0u) ? 0.0f : cascadeDistances[cascade - 1u];
	const F32 maxDist = cascadeDistances[cascade];

	factor = (distance - minDist) / max(kEpsilonF32, maxDist - minDist);
	factor = pow(factor, 16.0f);

	return UVec2(cascade, nextCascade);
}
