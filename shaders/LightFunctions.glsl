// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#ifndef ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL
#define ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL

#include "shaders/Common.glsl"

const float ATTENUATION_BOOST = 0.05;
const float OMNI_LIGHT_FRUSTUM_NEAR_PLANE = 0.1 / 4.0;

const uint SHADOW_SAMPLE_COUNT = 16;

//==============================================================================
// Get element count attached in a cluster
void getClusterInfo(in uint clusterIdx,
	out uint indexOffset,
	out uint pointLightCount,
	out uint spotLightCount,
	out uint probeCount)
{
	uint cluster = u_clusters[clusterIdx];
	indexOffset = cluster >> 16u;
	probeCount = (cluster >> 8u) & 0xFu;
	pointLightCount = (cluster >> 4u) & 0xFu;
	spotLightCount = cluster & 0xFu;
}

//==============================================================================
float computeAttenuationFactor(float lightRadius, vec3 frag2Light)
{
	float fragLightDist = dot(frag2Light, frag2Light);
	float att = 1.0 - fragLightDist * lightRadius;
	att = max(0.0, att);
	return att * att;
}

//==============================================================================
// Performs BRDF specular lighting
vec3 computeSpecularColorBrdf(vec3 v, // view dir
	vec3 l, // light dir
	vec3 n, // normal
	vec3 specCol,
	vec3 lightSpecCol,
	float a2, // rougness^2
	float nol) // N dot L
{
	vec3 h = normalize(l + v);

	// Fresnel
	float voh = dot(v, h);
#if 0
	// Schlick
	vec3 F = specCol + (1.0 - specCol) * pow((1.0 + EPSILON - loh), 5.0);
#else
	// Unreal
	vec3 F =
		specCol + (1.0 - specCol) * pow(2.0, (-5.55473 * voh - 6.98316) * voh);
#endif

	// D(n,h) aka NDF: GGX Trowbridge-Reitz
	float noh = dot(n, h);
	float D = noh * noh * (a2 - 1.0) + 1.0;
	D = a2 / (PI * D * D);
	D = clamp(D, EPSILON, 100.0); // Limit that because it may grow

// G(l,v,h)/(4*dot(n,h)*dot(n,v)) aka Visibility term: Geometric shadowing
// divided by BRDF denominator
#if 0
	float nov = max(EPSILON, dot(n, v));
	float V_v = nov + sqrt((nov - nov * a2) * nov + a2);
	float V_l = nol + sqrt((nol - nol * a2) * nol + a2);
	float V = 1.0 / (V_l * V_v);
#else
	float k = (a2 + 1.0);
	k = k * k / 8.0;
	float nov = max(EPSILON, dot(n, v));
	float V_v = nov * (1.0 - k) + k;
	float V_l = nol * (1.0 - k) + k;
	float V = 1.0 / (4.0 * V_l * V_v);
#endif

	return F * (V * D) * lightSpecCol;
}

//==============================================================================
vec3 computeDiffuseColor(vec3 diffCol, vec3 lightDiffCol)
{
	return diffCol * lightDiffCol;
}

//==============================================================================
float computeSpotFactor(vec3 l, float outerCos, float innerCos, vec3 spotDir)
{
	float costheta = -dot(l, spotDir);
	float spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

//==============================================================================
uint computeShadowSampleCount(const uint COUNT, float zVSpace)
{
	const float MAX_DISTANCE = 5.0;

	float z = max(zVSpace, -MAX_DISTANCE);
	float sampleCountf = float(COUNT) + z * (float(COUNT) / MAX_DISTANCE);
	sampleCountf = max(sampleCountf, 1.0);
	uint sampleCount = uint(sampleCountf);

	return sampleCount;
}

//==============================================================================
float computeShadowFactorSpot(
	mat4 lightProjectionMat, vec3 fragPos, float layer, uint sampleCount)
{
	vec4 texCoords4 = lightProjectionMat * vec4(fragPos, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if POISSON == 1
	const vec2 poissonDisk[SHADOW_SAMPLE_COUNT] =
		vec2[](vec2(0.751688, 0.619709) * 2.0 - 1.0,
			vec2(0.604741, 0.778485) * 2.0 - 1.0,
			vec2(0.936216, 0.463094) * 2.0 - 1.0,
			vec2(0.808758, 0.284966) * 2.0 - 1.0,
			vec2(0.812927, 0.786332) * 2.0 - 1.0,
			vec2(0.608651, 0.303919) * 2.0 - 1.0,
			vec2(0.482117, 0.573285) * 2.0 - 1.0,
			vec2(0.55819, 0.988451) * 2.0 - 1.0,
			vec2(0.340001, 0.728732) * 2.0 - 1.0,
			vec2(0.681775, 0.119789) * 2.0 - 1.0,
			vec2(0.217429, 0.522558) * 2.0 - 1.0,
			vec2(0.384257, 0.352163) * 2.0 - 1.0,
			vec2(0.143769, 0.738606) * 2.0 - 1.0,
			vec2(0.383474, 0.910019) * 2.0 - 1.0,
			vec2(0.409305, 0.177022) * 2.0 - 1.0,
			vec2(0.158647, 0.239097) * 2.0 - 1.0);

	float shadowFactor = 0.0;

	vec2 cordpart0 = vec2(layer, texCoords3.z);

	for(uint i = 0; i < sampleCount; i++)
	{
		vec2 cordpart1 = texCoords3.xy + poissonDisk[i] / 512.0;
		vec4 tcoord = vec4(cordpart1, cordpart0);

		shadowFactor += texture(u_spotMapArr, tcoord);
	}

	return shadowFactor / float(sampleCount);
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(u_spotMapArr, tcoord);

	return shadowFactor;
#endif
}

//==============================================================================
float computeShadowFactorOmni(vec3 frag2Light, float layer, float radius)
{
	vec3 dir = (u_lightingUniforms.viewMat * vec4(-frag2Light, 1.0)).xyz;
	vec3 dirabs = abs(dir);
	float dist = -max(dirabs.x, max(dirabs.y, dirabs.z));
	dir = normalize(dir);

	const float near = OMNI_LIGHT_FRUSTUM_NEAR_PLANE;
	const float far = radius;

	// Original code:
	// float g = near - far;
	// float z = (far + near) / g * dist + (2.0 * far * near) / g;
	// float w = -dist;
	// z /= w;
	// z = z * 0.5 + 0.5;
	// Optimized:
	float z = (far * (dist + near)) / (dist * (far - near));

	float shadowFactor = texture(u_omniMapArr, vec4(dir, layer), z).r;
	return shadowFactor;
}

//==============================================================================
// Compute the cubemap texture lookup vector given the reflection vector (r)
// the radius squared of the probe (R2) and the frag pos in sphere space (f)
vec3 computeCubemapVecAccurate(in vec3 r, in float R2, in vec3 f)
{
	// Compute the collision of the r to the inner part of the sphere
	// From now on we work on the sphere's space

	// Project the center of the sphere (it's zero now since we are in sphere
	// space) in ray "f,r"
	vec3 p = f - r * dot(f, r);

	// The collision to the sphere is point x where x = p + T * r
	// Because of the pythagorean theorem: R^2 = dot(p, p) + dot(T * r, T * r)
	// solving for T, T = R / |p|
	// then x becomes x = sqrt(R^2 - dot(p, p)) * r + p;
	float pp = dot(p, p);
	pp = min(pp, R2);
	float sq = sqrt(R2 - pp);
	vec3 x = p + sq * r;

	// Rotate UV to move it to world space
	vec3 uv = u_lightingUniforms.invViewRotation * x;

	return uv;
}

//==============================================================================
// Cheap version of computeCubemapVecAccurate
vec3 computeCubemapVecCheap(in vec3 r, in float R2, in vec3 f)
{
	return u_lightingUniforms.invViewRotation * r;
}

//==============================================================================
void readIndirect(in uint indexOffset,
	in uint probeCount,
	in vec3 posVSpace,
	in vec3 r,
	in vec3 n,
	in float lod,
	out vec3 specIndirect,
	out vec3 diffIndirect)
{
	specIndirect = vec3(0.0);
	diffIndirect = vec3(0.0);

	// Check proxy
	for(uint i = 0; i < probeCount; ++i)
	{
		uint probeIndex = u_lightIndices[indexOffset++];
		ReflectionProbe probe = u_reflectionProbes[probeIndex];

		float R2 = probe.positionRadiusSq.w;
		vec3 center = probe.positionRadiusSq.xyz;

		// Get distance from the center of the probe
		vec3 f = posVSpace - center;

		// Cubemap UV in view space
		vec3 uv = computeCubemapVecAccurate(r, R2, f);

		// Read!
		float cubemapIndex = probe.cubemapIndexPad3.x;
		vec3 c = textureLod(u_reflectionsTex, vec4(uv, cubemapIndex), lod).rgb;

		// Combine (lerp) with previous color
		float d = dot(f, f);
		float factor = d / R2;
		factor = min(factor, 1.0);
		specIndirect = mix(c, specIndirect, factor);
		// Same as: specIndirect = c * (1.0 - factor) + specIndirect * factor

		// Do the same for diffuse
		uv = computeCubemapVecCheap(n, R2, f);
		vec3 id = texture(u_irradianceTex, vec4(uv, cubemapIndex)).rgb;
		diffIndirect = mix(id, diffIndirect, factor);
	}
}

#endif
