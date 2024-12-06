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
template<typename T>
T V_Schlick(T roughness, T NoV, T NoL)
{
	const T k = (roughness * roughness) * T(0.5);
	const T Vis_SchlickV = NoV * (T(1) - k) + k;
	const T Vis_SchlickL = NoL * (T(1) - k) + k;
	return T(0.25) / (Vis_SchlickV * Vis_SchlickL);
}

// Visibility term: Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
template<typename T>
T V_SmithGGXCorrelatedFast(T roughness, T NoV, T NoL)
{
	const T a = roughness * roughness;
	const T v = T(0.5) / lerp(T(2) * NoL * NoV, NoL + NoV, a);
	return saturate(v);
}

template<typename T>
T Fd_Lambert()
{
	return T(1.0 / kPi);
}

template<typename T>
vector<T, 3> diffuseLobe(vector<T, 3> diffuse)
{
	return diffuse * Fd_Lambert<T>();
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

template<typename T>
vector<T, 3> specularDFG(vector<T, 3> F0, T roughness, Texture2D<Vec4> integrationLut, SamplerState integrationLutSampler, T NoV)
{
	const vector<T, 2> envBRDF = integrationLut.SampleLevel(integrationLutSampler, vector<T, 2>(roughness, NoV), 0.0).xy;
	return lerp(envBRDF.xxx, envBRDF.yyy, F0);
}

template<typename T>
T computeSpotFactor(vector<T, 3> frag2Light, T outerCos, T innerCos, vector<T, 3> spotDir)
{
	const T costheta = -dot(frag2Light, spotDir);
	const T spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

// PCSS calculation. Can be visualized here for spot lights: https://www.desmos.com/calculator/l0viaopwbi
// and here for directional: https://www.desmos.com/calculator/0dh0ybqvv1
template<typename T>
struct Pcss
{
	SamplerState m_linearClampSampler;

	vector<T, 2> computePenumbra(Texture2D<Vec4> shadowmap, Vec2 searchDist, Vec3 projCoords, T cosTheta, T sinTheta, F32 lightSize, Bool dirLight)
	{
		T inShadowCount = 0.0;
		F32 avgOccluderZ = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const vector<T, 2> diskPoint = kPoissonDisk4[i];

			// Rotate the disk point
			vector<T, 2> rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = projCoords.xy + rotatedDiskPoint * searchDist;

			const F32 occluderZ = shadowmap.SampleLevel(m_linearClampSampler, newUv, 0.0).x;
			if(projCoords.z >= occluderZ)
			{
				inShadowCount += 1.0;
				avgOccluderZ += occluderZ;
			}
		}

		T factor;
		if(inShadowCount == 0.0 || inShadowCount == ARRAY_SIZE(kPoissonDisk4))
		{
			factor = 0.0;
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

		return vector<T, 2>(factor, inShadowCount);
	}
};

template<typename T>
struct PcssDisabled
{
	vector<T, 2> computePenumbra(Texture2D<Vec4> shadowmap, Vec2 searchDist, Vec3 projCoords, T cosTheta, T sinTheta, F32 lightSize, Bool dirLight)
	{
		return -1.0;
	}
};

template<typename T, typename TPcss>
T computeShadowFactorSpotLightGeneric(SpotLight light, Vec3 worldPos, Texture2D<Vec4> shadowTex, SamplerComparisonState shadowMapSampler, Bool pcf,
									  T randFactor, TPcss pcss)
{
	const Vec4 texCoords4 = mul(light.m_textureMatrix, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	T shadow;
	if(pcf)
	{
		Vec2 texSize;
		F32 mipCount;
		shadowTex.GetDimensions(0, texSize.x, texSize.y, mipCount);
		const Vec2 smTexelSize = 1.0 / texSize;

		const T sinTheta = sin(randFactor * 2.0 * kPi);
		const T cosTheta = cos(randFactor * 2.0 * kPi);

		// PCSS
		const vector<T, 2> pcssRes =
			pcss.computePenumbra(shadowTex, smTexelSize * kPcssSearchTexelRadius, texCoords3, cosTheta, sinTheta, light.m_radius, false);
		T pcfPixels;
		if(pcssRes.x == -1.0)
		{
			// PCSS disabled
			pcfPixels = kPcfTexelRadius;
		}
		else
		{
			if(pcssRes.y == ARRAY_SIZE(kPoissonDisk4))
			{
				return 0.0;
			}

			pcfPixels = kPcfTexelRadius + pcssRes.x * kPcssTexelRadius;
		}

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const vector<T, 2> diskPoint = kPoissonDisk4[i];

			// Rotate the disk point
			vector<T, 2> rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = texCoords3.xy + rotatedDiskPoint * smTexelSize * pcfPixels;

			shadow += shadowTex.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
		}

		shadow /= T(ARRAY_SIZE(kPoissonDisk4));
	}
	else
	{
		shadow = shadowTex.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
	}

	return shadow;
}

template<typename T>
T computeShadowFactorSpotLight(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler)
{
	PcssDisabled<T> noPcss = (PcssDisabled<T>)0;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, false, 0.0, noPcss);
}

template<typename T>
T computeShadowFactorSpotLightPcf(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler, T randFactor)
{
	PcssDisabled<T> noPcss = (PcssDisabled<T>)0;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, true, randFactor, noPcss);
}

template<typename T>
T computeShadowFactorSpotLightPcss(SpotLight light, Vec3 worldPos, Texture2D shadowTex, SamplerComparisonState shadowMapSampler, T randFactor,
								   SamplerState linearClampAnySampler)
{
	Pcss<T> pcss;
	pcss.m_linearClampSampler = linearClampAnySampler;
	return computeShadowFactorSpotLightGeneric(light, worldPos, shadowTex, shadowMapSampler, true, randFactor, pcss);
}

// Compute the shadow factor of point (omni) lights.
template<typename T>
T computeShadowFactorPointLightGeneric(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler, T randFactor,
									   Bool pcf)
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
	T shadow;
	if(pcf)
	{
		F32 mipCount;
		Vec2 smTexelSize;
		shadowMap.GetDimensions(0, smTexelSize.x, smTexelSize.y, mipCount);
		smTexelSize = 1.0 / smTexelSize;

		const T sinTheta = sin(randFactor * 2.0 * kPi);
		const T cosTheta = cos(randFactor * 2.0 * kPi);

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk4); ++i)
		{
			const vector<T, 2> diskPoint = kPoissonDisk4[i];

			// Rotate the disk point
			vector<T, 2> rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			const Vec2 newUv = uv + rotatedDiskPoint * smTexelSize * kPcfTexelRadius;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, z);
		}

		shadow /= T(ARRAY_SIZE(kPoissonDisk4));
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, uv, z);
	}

	return shadow;
}

template<typename T>
T computeShadowFactorPointLight(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, -1.0, false);
}

template<typename T>
T computeShadowFactorPointLightPcf(PointLight light, Vec3 frag2Light, Texture2D shadowMap, SamplerComparisonState shadowMapSampler, T randFactor)
{
	return computeShadowFactorPointLightGeneric(light, frag2Light, shadowMap, shadowMapSampler, randFactor, true);
}

// Compute the shadow factor of a directional light
template<typename T, typename TPcss>
T computeShadowFactorDirLightGeneric(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap,
									 SamplerComparisonState shadowMapSampler, T randFactor, Bool pcf, TPcss pcss)
{
#define ANKI_FAST_CASCADES_WORKAROUND 1 // light might be in a constant buffer and dynamic indexing in constant buffers is too slow on nvidia

#if ANKI_FAST_CASCADES_WORKAROUND
	// Assumes kMaxShadowCascades is 4
	Mat4 lightProjectionMat;
	F32 far;
	F32 pcfDistUvSpace;
	switch(cascadeIdx)
	{
	case 0:
		lightProjectionMat = light.m_textureMatrices[0];
		far = light.m_cascadeFarPlanes[0];
		pcfDistUvSpace = light.m_cascadePcfTexelRadius[0];
		break;
	case 1:
		lightProjectionMat = light.m_textureMatrices[1];
		far = light.m_cascadeFarPlanes[1];
		pcfDistUvSpace = light.m_cascadePcfTexelRadius[1];
		break;
	case 2:
		lightProjectionMat = light.m_textureMatrices[2];
		far = light.m_cascadeFarPlanes[2];
		pcfDistUvSpace = light.m_cascadePcfTexelRadius[2];
		break;
	default:
		lightProjectionMat = light.m_textureMatrices[3];
		far = light.m_cascadeFarPlanes[3];
		pcfDistUvSpace = light.m_cascadePcfTexelRadius[3];
	}
#else
	const Mat4 lightProjectionMat = light.m_textureMatrices[cascadeIdx];
#endif

	const Vec4 texCoords4 = mul(lightProjectionMat, Vec4(worldPos, 1.0));
	Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	T shadow;
	if(pcf || pcfDistUvSpace == 0.0f)
	{
		const T sinTheta = sin(randFactor * 2.0 * kPi);
		const T cosTheta = cos(randFactor * 2.0 * kPi);

		// PCSS
		const Vec2 pcssRes =
			pcss.computePenumbra(shadowMap, pcfDistUvSpace * (kPcssSearchTexelRadius / kPcfTexelRadius), texCoords3, cosTheta, sinTheta, far, true);
		if(pcssRes.x == -1.0)
		{
			// PCSS disabled, do nothing
		}
		else
		{
			if(pcssRes.y == ARRAY_SIZE(kPoissonDisk4))
			{
				return 0.0;
			}

			pcfDistUvSpace = pcfDistUvSpace + pcssRes.x * pcfDistUvSpace * (kPcssTexelRadius / kPcfTexelRadius);
		}

		shadow = 0.0;
		[unroll] for(U32 i = 0u; i < ARRAY_SIZE(kPoissonDisk8); ++i)
		{
			const vector<T, 2> diskPoint = kPoissonDisk8[i];

			// Rotate the disk point
			vector<T, 2> rotatedDiskPoint;
			rotatedDiskPoint.x = diskPoint.x * cosTheta - diskPoint.y * sinTheta;
			rotatedDiskPoint.y = diskPoint.y * cosTheta + diskPoint.x * sinTheta;

			// Offset calculation
			Vec2 newUv = texCoords3.xy + rotatedDiskPoint * pcfDistUvSpace;

			shadow += shadowMap.SampleCmpLevelZero(shadowMapSampler, newUv, texCoords3.z);
		}

		shadow /= T(ARRAY_SIZE(kPoissonDisk8));
	}
	else
	{
		shadow = shadowMap.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
	}

	return shadow;
}

template<typename T>
T computeShadowFactorDirLight(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap, SamplerComparisonState shadowMapSampler)
{
	PcssDisabled<T> noPcss = (PcssDisabled<T>)0;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, -1.0, false, noPcss);
}

template<typename T>
T computeShadowFactorDirLightPcf(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap, SamplerComparisonState shadowMapSampler,
								 T randFactor)
{
	PcssDisabled<T> noPcss = (PcssDisabled<T>)0;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, randFactor, true, noPcss);
}

template<typename T>
T computeShadowFactorDirLightPcss(DirectionalLight light, U32 cascadeIdx, Vec3 worldPos, Texture2D shadowMap, SamplerComparisonState shadowMapSampler,
								  T randFactor, SamplerState linearClampAnySampler)
{
	Pcss<T> pcss;
	pcss.m_linearClampSampler = linearClampAnySampler;
	return computeShadowFactorDirLightGeneric(light, cascadeIdx, worldPos, shadowMap, shadowMapSampler, randFactor, true, pcss);
}

// Compute the shadow factor of a directional light
template<typename T>
T computeShadowFactorDirLight(Mat4 lightProjectionMat, Vec3 worldPos, Texture2D<Vec4> shadowMap, SamplerComparisonState shadowMapSampler)
{
	const Vec4 texCoords4 = mul(lightProjectionMat, Vec4(worldPos, 1.0));
	const Vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const T shadowFactor = shadowMap.SampleCmpLevelZero(shadowMapSampler, texCoords3.xy, texCoords3.z);
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

template<typename T>
T computeAttenuationFactor(T lightRadius, Vec3 frag2Light)
{
	const F32 fragLightDist = dot(frag2Light, frag2Light);
	T att = fragLightDist / (lightRadius * lightRadius);
	att = T(1) - att;
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
template<typename T>
vector<T, 3> sampleAmbientDice(vector<T, 3> posx, vector<T, 3> negx, vector<T, 3> posy, vector<T, 3> negy, vector<T, 3> posz, vector<T, 3> negz,
							   vector<T, 3> normal)
{
	normal.z *= -1.0;
	const vector<T, 3> axisWeights = normal * normal;
	const vector<T, 3> uv = normal * 0.5 + 0.5;

	vector<T, 3> col = lerp(negx, posx, uv.x) * axisWeights.x;
	col += lerp(negy, posy, uv.y) * axisWeights.y;
	col += lerp(negz, posz, uv.z) * axisWeights.z;

	// Divide by weight
	col /= axisWeights.x + axisWeights.y + axisWeights.z + 0.0001;

	return col;
}

// Sample the irradiance term from the clipmap
template<typename T>
vector<T, 3> sampleGlobalIllumination(const Vec3 worldPos, const vector<T, 3> normal, const GlobalIlluminationProbe probe, Texture3D<Vec4> tex,
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
	vector<T, 3> irradiancePerDir[6u];
	[unroll] for(U32 dir = 0u; dir < 6u; ++dir)
	{
		// Point to the correct UV
		Vec3 shiftedUVw = uvw;
		shiftedUVw.x += (1.0 / 6.0) * F32(dir);

		irradiancePerDir[dir] = tex.SampleLevel(linearAnyClampSampler, shiftedUVw, 0.0).rgb;
	}

	// Sample the irradiance
	const vector<T, 3> irradiance = sampleAmbientDice<T>(irradiancePerDir[0], irradiancePerDir[1], irradiancePerDir[2], irradiancePerDir[3],
														 irradiancePerDir[4], irradiancePerDir[5], normal);

	return irradiance;
}

/// To play with it use https://www.shadertoy.com/view/sttSDf
/// http://jcgt.org/published/0007/04/01/paper.pdf by Eric Heitz
/// Input v: view direction (camPos - pos)
/// Input alphaX, alphaY: roughness parameters
/// Input u1, u2: uniform random numbers
/// Output: normal sampled with PDF D_Ve(nE) = G1(v) * max(0, dot(v, nE)) * D(nE) / v.z
Vec3 sampleGgxVndf(Vec3 v, F32 alphaX, F32 alphaY, F32 u1, F32 u2)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	const Vec3 vH = normalize(Vec3(alphaX * v.x, alphaY * v.y, v.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	const F32 lensq = vH.x * vH.x + vH.y * vH.y;
	const Vec3 tangent1 = (lensq > 0.0) ? Vec3(-vH.y, vH.x, 0) * rsqrt(lensq) : Vec3(1.0, 0.0, 0.0);
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

// The PDF for the sampleGgxVndf. It's D_Ve(nE) = G1(v) * max(0, dot(v, nE)) * D(nE) / v.z
F32 pdfGgxVndf(Vec3 nE, Vec3 v, F32 alphaX, F32 alphaY)
{
	// Equation (2) in the paper
	F32 lambdaV = (pow2(alphaX * v.x) + pow2(alphaY * v.y)) / pow2(v.z);
	lambdaV = (-1.0 + sqrt(1.0 + lambdaV)) / 2.0;
	F32 G1 = 1.0 / (1.0 + lambdaV);

	// Equation (1) in the paper
	F32 DnE = pow2(nE.x / alphaX) + pow2(nE.y / alphaY) + pow2(nE.z);
	DnE = kPi * alphaX * alphaY * pow2(DnE);
	DnE = 1.0 / DnE;

	const F32 pdf = G1 * max(0.0, dot(v, nE)) * DnE / v.z;
	return pdf;
}

// Same thing as sampleGgxVndf but it works in world space and not in TBN and it's also isotropic
// https://auzaiffe.wordpress.com/2024/04/15/vndf-importance-sampling-an-isotropic-distribution/
// viewDir is camPos-worldPos
Vec3 sampleVndfIsotropic(Vec2 randFactors, Vec3 viewDir, F32 alpha, Vec3 normal)
{
	// decompose the floattor in parallel and perpendicular components
	const Vec3 viewDirZ = -normal * dot(viewDir, normal);
	const Vec3 viewDirXY = viewDir + viewDirZ;

	// warp to the hemisphere configuration
	const Vec3 wiStd = -normalize(alpha * viewDirXY + viewDirZ);

	// sample a spherical cap in (-wiStd.z, 1]
	const F32 wiStdZ = dot(wiStd, normal);
	const F32 z = 1.0 - randFactors.y * (1.0 + wiStdZ);
	const F32 sinTheta = sqrt(saturate(1.0 - z * z));
	const F32 phi = 2.0 * kPi * randFactors.x - kPi;
	const F32 x = sinTheta * cos(phi);
	const F32 y = sinTheta * sin(phi);
	const Vec3 cStd = Vec3(x, y, z);

	// reflect sample to align with normal
	const Vec3 up = Vec3(0, 0, 1.000001); // Used for the singularity
	const Vec3 wr = normal + up;
	const Vec3 c = dot(wr, cStd) * wr / wr.z - cStd;

	// compute halfway direction as standard normal
	const Vec3 wmStd = c + wiStd;
	const Vec3 wmStdZ = normal * dot(normal, wmStd);
	const Vec3 wmStdXY = wmStdZ - wmStd;

	// return final normal
	const Vec3 nE = normalize(alpha * wmStdXY + wmStdZ);
	return nE;
}

// The PDF of sampleVndfIsotropic
F32 pdfVndfIsotropic(Vec3 reflectedDir, Vec3 viewDir, F32 alpha, Vec3 normal)
{
	const F32 alphaSquare = alpha * alpha;
	const Vec3 wm = normalize(reflectedDir + viewDir);
	const F32 zm = dot(wm, normal);
	const F32 zi = dot(viewDir, normal);
	const F32 nrm = rsqrt((zi * zi) * (1.0f - alphaSquare) + alphaSquare);
	const F32 sigmaStd = (zi * nrm) * 0.5f + 0.5f;
	const F32 sigmaI = sigmaStd / nrm;
	const F32 nrmN = (zm * zm) * (alphaSquare - 1.0f) + 1.0f;
	return alphaSquare / (kPi * 4.0f * nrmN * nrmN * sigmaI);
}

/// Calculate the reflection vector based on roughness. Sometimes the refl vector is bellow the normal so this func will try again to get a new one.
/// viewDir is camPos-worldPos
Vec3 sampleReflectionVectorAnisotropic(Vec3 viewDir, Vec3 normal, F32 roughnessX, F32 roughnessY, Vec2 randFactors, U32 tryCount, out F32 pdf)
{
	pdf = 0.0;
	const Mat3 tbn = rotationFromDirection(normal);
	const Mat3 tbnT = transpose(tbn);
	const Vec3 viewDirTbn = mul(tbnT, viewDir);

	const F32 alphaX = roughnessX * roughnessX;
	const F32 alphaY = roughnessY * roughnessY;

	Vec3 reflectedDirTbn;
	do
	{
		const Vec3 sampledNormalTbn = sampleGgxVndf(viewDirTbn, alphaX, alphaY, randFactors.x, randFactors.y);
		reflectedDirTbn = reflect(-viewDirTbn, sampledNormalTbn);

		if(dot(reflectedDirTbn, Vec3(0.0, 0.0, 1.0)) > cos(kPi / 2.0 * 0.9))
		{
			// Angle between the refl vec and the normal is less than 90 degr. We are good to go
			pdf = pdfGgxVndf(sampledNormalTbn, viewDirTbn, alphaX, alphaY);
			break;
		}
		else
		{
			// Try again
			randFactors.x = frac(randFactors.x + 0.7324);
			randFactors.y = frac(randFactors.y + 0.6523);
		}
	} while(--tryCount);

	// Transform reflectedDirTbn back to the initial space.
	const Vec3 r = mul(tbn, reflectedDirTbn);

	return r;
}

// Another version of sampleReflectionVector. Possibly faster
Vec3 sampleReflectionVectorIsotropic(Vec3 viewDir, Vec3 normal, F32 roughness, Vec2 randFactors, U32 tryCount, out F32 pdf)
{
	const F32 alpha = roughness * roughness;

	Vec3 reflDir = normal;
	do
	{
		const Vec3 nE = sampleVndfIsotropic(randFactors, viewDir, alpha, normal);
		reflDir = reflect(-viewDir, nE);

		if(dot(reflDir, normal) > cos(kPi / 2.0 * 0.9))
		{
			// Angle between the refl vec and the normal is less than 90 degr. We are good to go
			break;
		}
		else
		{
			// Try again
			randFactors.x = frac(randFactors.x + 0.7324);
			randFactors.y = frac(randFactors.y + 0.6523);
		}
	} while(--tryCount);

	pdf = pdfVndfIsotropic(reflDir, viewDir, alpha, normal);
	return reflDir;
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
template<typename T>
UVec2 computeShadowCascadeIndex2(F32 distance, Vec4 cascadeDistances, U32 shadowCascadeCount, out T factor)
{
	const U32 cascade = computeShadowCascadeIndex(distance, cascadeDistances, shadowCascadeCount);
	const U32 nextCascade = min(cascade + 1u, shadowCascadeCount - 1u);

	const F32 minDist = (cascade == 0u) ? 0.0 : cascadeDistances[cascade - 1u];
	const F32 maxDist = cascadeDistances[cascade];

	factor = (distance - minDist) / max(kEpsilonF32, maxDist - minDist);
	factor = pow(factor, T(16.0)); // WARNING: Need to change the C++ code if you change this

	return UVec2(cascade, nextCascade);
}
