// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#pragma once

#include <AnKi/Shaders/Functions.hlsl>
#include <AnKi/Shaders/VisibilityAndCollisionFunctions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Shaders/Include/MiscRendererTypes.h>

constexpr Vec2 kPoissonDisk4[4u] = {Vec2(-0.15, 0.06), Vec2(0.14, -0.48), Vec2(-0.05, 0.97), Vec2(0.58, -0.18)};

constexpr Vec2 kPoissonDisk8[8u] = {Vec2(-0.16, 0.66), Vec2(0.15, -0.02), Vec2(-0.76, 0.49),  Vec2(-0.57, -0.48),
									Vec2(0.82, -0.43), Vec2(0.91, 0.15),  Vec2(-0.39, -0.86), Vec2(0.24, -0.76)};

constexpr RF32 kPcfScale = 2.0f;
constexpr RF32 kPcssScale = kPcfScale * 12.0;
constexpr RF32 kPcssSearchScale = kPcfScale * 12.0;
constexpr F32 kPcssDirLightMaxPenumbraMeters = 6.0; // If the occluder and the reciever have more than this value then do full penumbra

// Fresnel term unreal
// specular: The specular color aka F0
Vec3 F_Unreal(Vec3 specular, F32 VoH)
{
	return specular + (1.0 - specular) * pow(2.0, (-5.55473 * VoH - 6.98316) * VoH);
}

// Fresnel Schlick: "An Inexpensive BRDF Model for Physically-Based Rendering"
// It has lower VGRPs than F_Unreal
template<typename T>
vector<T, 3> F_Schlick(vector<T, 3> f0, T VoH)
{
	const T f = pow(max(T(0), T(1) - VoH), T(5.0));
	return f + f0 * (T(1) - f);
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
template<typename T>
T D_GGX(T roughness, T NoH, vector<T, 3> h, vector<T, 3> worldNormal)
{
#if 0 && ANKI_PLATFORM_MOBILE
	const vector<T, 3> NxH = cross(worldNormal, h);
	const T oneMinusNoHSquared = dot(NxH, NxH);
#else
	const T oneMinusNoHSquared = T(1) - NoH * NoH;
	ANKI_MAYBE_UNUSED(h);
	ANKI_MAYBE_UNUSED(worldNormal);
#endif

	const T a = roughness * roughness;
	const T v = NoH * a;
	const T k = a / (oneMinusNoHSquared + v * v);
	const T d = k * k * T(1.0 / kPi);
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
template<typename T>
T V_SmithGGXCorrelatedFast(T roughness, T NoV, T NoL)
{
	const T a = roughness * roughness;
	const T v = T(0.5) / lerp(T(2) * NoL * NoV, NoL + NoV, a);
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
template<typename T>
vector<T, 3> specularIsotropicLobe(vector<T, 3> normal, vector<T, 3> f0, T roughness, vector<T, 3> viewDir, vector<T, 3> frag2Light)
{
	const vector<T, 3> H = normalize(frag2Light + viewDir);

	const T NoL = max(0.0, dot(normal, frag2Light));
	const T VoH = max(0.0, dot(viewDir, H));
	const T NoH = max(0.0, dot(normal, H));
	const T NoV = max(0.05, dot(normal, viewDir));

	// F
	const vector<T, 3> F = F_Schlick(f0, VoH);

	// D
	const T D = D_GGX(roughness, NoH, H, normal);

	// Vis
	const T V = V_SmithGGXCorrelatedFast(roughness, NoV, NoL);

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

// PCSS calculation. Can be visualized here for spot lights: https://www.desmos.com/calculator/l0viaopwbi
// and here for directional: https://www.desmos.com/calculator/0dh0ybqvv1
struct Pcss
{
	SamplerState m_linearClampSampler;

	Vec2 computePenumbra(Texture2D<Vec4> shadowmap, Vec2 smTexelSize, Vec3 projCoords, F32 cosTheta, F32 sinTheta, F32 lightSize, Bool dirLight)
	{
		F32 inShadowCount = 0.0;
		F32 avgOccluderZ = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const Vec2 diskPoint = kPoissonDisk4[i] * kPcssSearchScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = projCoords.xy + rotatedDiskPoint * smTexelSize;

			const F32 occluderZ = shadowmap.SampleLevel(m_linearClampSampler, newUv, 0.0).x;
			if(projCoords.z >= occluderZ)
			{
				inShadowCount += 1.0;
				avgOccluderZ += occluderZ;
			}
		}

		F32 factor;
		if(inShadowCount == 0.0 || inShadowCount == ARRAY_SIZE(kPoissonDisk4))
		{
			factor = 0;
		}
		else
		{
			avgOccluderZ /= inShadowCount;
			if(!dirLight)
			{
				factor = (projCoords.z - avgOccluderZ) * lightSize / avgOccluderZ;
			}
			else
			{
				// Dir light's depth is linear
				factor = (projCoords.z - avgOccluderZ) * lightSize / kPcssDirLightMaxPenumbraMeters;
				factor *= factor;
			}
		}

		return Vec2(factor, inShadowCount);
	}
};

struct PcssDisabled
{
	Vec2 computePenumbra(Texture2D<Vec4> shadowmap, Vec2 texelSize, Vec3 projCoords, F32 cosTheta, F32 sinTheta, F32 lightSize, Bool dirLight)
	{
		return -1.0;
	}
};

template<typename TPcss>
RF32 computeShadowFactorSpotLightGeneric(SpotLight light, Vec3 worldPos, Texture2D<Vec4> shadowTex, SamplerComparisonState shadowMapSampler, Bool pcf,
										 RF32 randFactor, TPcss pcss)
{
	const Vec4 texCoords4 = mul(light.m_textureMatrix, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	RF32 shadow;
	if(pcf)
	{
		Vec2 texSize;
		F32 mipCount;
		shadowTex.GetDimensions(0, texSize.x, texSize.y, mipCount);
		const Vec2 smTexelSize = 1.0 / texSize;

		const F32 sinTheta = sin(randFactor * 2.0 * kPi);
		const F32 cosTheta = cos(randFactor * 2.0 * kPi);

		// PCSS
		const Vec2 pcssRes = pcss.computePenumbra(shadowTex, smTexelSize, texCoords3, cosTheta, sinTheta, light.m_radius, false);
		F32 pcsfScale;
		if(pcssRes.x == -1.0)
		{
			// PCSS disabled
			pcsfScale = kPcfScale;
		}
		else
		{
			if(pcssRes.y == ARRAY_SIZE(kPoissonDisk4))
			{
				return 0.0;
			}

			pcsfScale = kPcfScale + pcssRes.x * kPcssScale;
		}

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const Vec2 diskPoint = kPoissonDisk4[i] * pcsfScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = texCoords3.xy + rotatedDiskPoint * smTexelSize;

			shadow += shadowTex.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
		}

		shadow /= F32(ARRAY_SIZE(kPoissonDisk4));
	}
	else
	{
		shadow = shadowTex.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
	}

	return shadow;
}

RF32 computeShadowFactorSpotLight(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler)
{
	PcssDisabled noPcss = (PcssDisabled)0;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, false, 0.0, noPcss);
}

RF32 computeShadowFactorSpotLightPcf(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler, RF32 randFactor)
{
	PcssDisabled noPcss = (PcssDisabled)0;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, true, randFactor, noPcss);
}

RF32 computeShadowFactorSpotLightPcss(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler, RF32 randFactor,
									  SamplerState linearClampAnySampler)
{
	Pcss pcss;
	pcss.m_linearClampSampler = linearClampAnySampler;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, true, randFactor, pcss);
}

// Compute the shadow factor of point (omni) lights.
RF32 computeShadowFactorPointLightGeneric(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler,
										  RF32 randFactor, Bool pcf)
{
	const Vec3 dir = -frag2Light;
	const Vec3 dirabs = abs(dir);
	const F32 dist = max(dirabs.x, max(dirabs.y, dirabs.z)) - 0.01; // Push it out to avoid artifacts

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
	Vec2 uv = convertCubeUvs(dir * Vec3(1.0, 1.0, -1.0), faceIdxu);

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
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const Vec2 diskPoint = kPoissonDisk4[i] * kPcfScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = uv + rotatedDiskPoint * smTexelSize;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, z);
		}

		shadow /= F32(ARRAY_SIZE(kPoissonDisk4));
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, uv, z);
	}

	return shadow;
}

RF32 computeShadowFactorPointLight(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, -1.0, false);
}

RF32 computeShadowFactorPointLightPcf(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler,
									  RF32 randFactor)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, randFactor, true);
}

// Compute the shadow factor of a directional light
template<typename TPcss>
RF32 computeShadowFactorDirLightGeneric(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
										SamplerComparisonState shadowMapSampler, RF32 randFactor, Bool pcf, TPcss pcss)
{
#define ANKI_FAST_CASCADES_WORKAROUND 1 // light might be in a constant buffer and dynamic indexing in constant buffers is too slow on nvidia

#if ANKI_FAST_CASCADES_WORKAROUND
	// Assumes kMaxShadowCascades is 4
	Mat4 lightProjectionMat;
	F32 far;
	switch(cascadeIdx)
	{
	case 0:
		lightProjectionMat = light.m_textureMatrices[0];
		far = light.m_cascadeFarPlanes[0];
		break;
	case 1:
		lightProjectionMat = light.m_textureMatrices[1];
		far = light.m_cascadeFarPlanes[1];
		break;
	case 2:
		lightProjectionMat = light.m_textureMatrices[2];
		far = light.m_cascadeFarPlanes[2];
		break;
	default:
		lightProjectionMat = light.m_textureMatrices[3];
		far = light.m_cascadeFarPlanes[3];
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

		// PCSS
		const Vec2 pcssRes = pcss.computePenumbra(shadowMap, smTexelSize, texCoords3, cosTheta, sinTheta, far, true);
		F32 pcsfScale;
		if(pcssRes.x == -1.0)
		{
			// PCSS disabled
			pcsfScale = kPcfScale;
		}
		else
		{
			if(pcssRes.y == ARRAY_SIZE(kPoissonDisk4))
			{
				return 0.0;
			}

			pcsfScale = kPcfScale + pcssRes.x * kPcssScale;
		}

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk8); ++i)
		{
			const Vec2 diskPoint = kPoissonDisk8[i] * pcsfScale;

			// Rotate the disk point
			Vec2 rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			Vec2 newUv = texCoords3.xy + rotatedDiskPoint * smTexelSize;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
		}

		shadow /= F32(ARRAY_SIZE(kPoissonDisk8));
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
	}

	return shadow;
}

RF32 computeShadowFactorDirLight(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap, SamplerComparisonState shadowMapSampler)
{
	PcssDisabled noPcss = (PcssDisabled)0;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, -1.0, false, noPcss);
}

RF32 computeShadowFactorDirLightPcf(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
									SamplerComparisonState shadowMapSampler, F32 randFactor)
{
	PcssDisabled noPcss = (PcssDisabled)0;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, randFactor, true, noPcss);
}

RF32 computeShadowFactorDirLightPcss(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
									 SamplerComparisonState shadowMapSampler, F32 randFactor, SamplerState linearClampAnySampler)
{
	Pcss pcss;
	pcss.m_linearClampSampler = linearClampAnySampler;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, randFactor, true, pcss);
}

// Compute the shadow factor of a directional light
RF32 computeShadowFactorDirLight(Mat4 lightProjectionMat, Vec3 worldPos, Texture2D<RVec4> shadowMap, SamplerComparisonState shadowMapSampler)
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

RF32 computeAttenuationFactor(RF32 lightRadius, RVec3 frag2Light)
{
	const RF32 fragLightDist = dot(frag2Light, frag2Light);
	RF32 att = 1.0 - fragLightDist / (lightRadius * lightRadius);
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
	normal.z *= -1.0f;
	const RVec3 axisWeights = normal * normal;
	const RVec3 uv = normal * 0.5f + 0.5f;

	RVec3 col = lerp(negx, posx, uv.x) * axisWeights.x;
	col += lerp(negy, posy, uv.y) * axisWeights.y;
	col += lerp(negz, posz, uv.z) * axisWeights.z;

	// Divide by weight
	col /= axisWeights.x + axisWeights.y + axisWeights.z + kEpsilonRF32;

	return col;
}

// Sample the irradiance term from the clipmap
RVec3 sampleGlobalIllumination(const Vec3 worldPos, const Vec3 normal, const GlobalIlluminationProbe probe, Texture3D<RVec4> tex,
							   SamplerState linearAnyClampSampler)
{
	// Find the UVW
	Vec3 uvw = (worldPos - probe.m_aabbMin) / (probe.m_aabbMax - probe.m_aabbMin);
	uvw = saturate(uvw);
	uvw.y = 1.0f - uvw.y;

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

		irradiancePerDir[dir] = tex.SampleLevel(linearAnyClampSampler, shiftedUVw, 0.0).rgb;
	}

	// Sample the irradiance
	const RVec3 irradiance = sampleAmbientDice(irradiancePerDir[0], irradiancePerDir[1], irradiancePerDir[2], irradiancePerDir[3],
											   irradiancePerDir[4], irradiancePerDir[5], normal);

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
	factor = pow(factor, 16.0f); // WARNING: Need to change the C++ code if you change this

	return UVec2(cascade, nextCascade);
}
