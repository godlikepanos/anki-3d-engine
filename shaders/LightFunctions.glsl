// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#ifndef ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL
#define ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL

#include "shaders/Functions.glsl"

const float LIGHT_FRUSTUM_NEAR_PLANE = 0.1 / 4.0;
const uint SHADOW_SAMPLE_COUNT = 16;
const float ESM_CONSTANT = 40.0;

float computeAttenuationFactor(float lightRadius, vec3 frag2Light)
{
	float fragLightDist = dot(frag2Light, frag2Light);
	float att = 1.0 - fragLightDist * lightRadius;
	att = max(0.0, att);
	return att * att;
}

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
	vec3 F = specCol + (1.0 - specCol) * pow(2.0, (-5.55473 * voh - 6.98316) * voh);
#endif

	// D(n,h) aka NDF: GGX Trowbridge-Reitz
	float noh = dot(n, h);
	float D = noh * noh * (a2 - 1.0) + 1.0;
	D = a2 / (PI * D * D);

// G(l,v,h)/(4*dot(n,h)*dot(n,v)) aka Visibility term: Geometric shadowing divided by BRDF denominator
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

vec3 computeDiffuseColor(vec3 diffCol, vec3 lightDiffCol)
{
	return diffCol * lightDiffCol;
}

float computeSpotFactor(vec3 l, float outerCos, float innerCos, vec3 spotDir)
{
	float costheta = -dot(l, spotDir);
	float spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

uint computeShadowSampleCount(const uint COUNT, float zVSpace)
{
	const float MAX_DISTANCE = 5.0;

	float z = max(zVSpace, -MAX_DISTANCE);
	float sampleCountf = float(COUNT) + z * (float(COUNT) / MAX_DISTANCE);
	sampleCountf = max(sampleCountf, 1.0);
	uint sampleCount = uint(sampleCountf);

	return sampleCount;
}

float computeShadowFactorSpot(mat4 lightProjectionMat, vec3 worldPos, float distance, sampler2D spotMapArr)
{
	vec4 texCoords4 = lightProjectionMat * vec4(worldPos, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

	const float near = LIGHT_FRUSTUM_NEAR_PLANE;
	const float far = distance;

	float linearDepth = linearizeDepth(texCoords3.z, near, far);

	return clamp(exp(ESM_CONSTANT * (textureLod(spotMapArr, texCoords3.xy, 0.0).r - linearDepth)), 0.0, 1.0);
}

float computeShadowFactorOmni(vec3 frag2Light, float radius, uvec2 atlasTiles, float tileSize, sampler2D shadowMap)
{
	vec3 dir = -frag2Light;
	vec3 dirabs = abs(dir);
	float dist = max(dirabs.x, max(dirabs.y, dirabs.z));

	const float near = LIGHT_FRUSTUM_NEAR_PLANE;
	const float far = radius;

	float linearDepth = (dist - near) / (far - near);

	// Read tex
	float shadowFactor;
	{
		// Convert cube coords
		uint faceIdxu;
		vec2 uv = convertCubeUvsu(dir, faceIdxu);

		// Compute atlas tile
		atlasTiles >>= uvec2(faceIdxu * 5u);
		atlasTiles &= uvec2(31u);

		// Compute UV
		uv = (uv + vec2(atlasTiles)) * tileSize;

		// Sample
		shadowFactor = textureLod(shadowMap, uv, 0.0).r;
	}

	return clamp(exp(ESM_CONSTANT * (shadowFactor - linearDepth)), 0.0, 1.0);
}

// Compute the cubemap texture lookup vector given the reflection vector (r) the radius squared of the probe (R2) and
// the frag pos in sphere space (f)
vec3 computeCubemapVecAccurate(in vec3 r, in float R2, in vec3 f)
{
	// Compute the collision of the r to the inner part of the sphere
	// From now on we work on the sphere's space

	// Project the center of the sphere (it's zero now since we are in sphere space) in ray "f,r"
	vec3 p = f - r * dot(f, r);

	// The collision to the sphere is point x where x = p + T * r
	// Because of the pythagorean theorem: R^2 = dot(p, p) + dot(T * r, T * r)
	// solving for T, T = R / |p|
	// then x becomes x = sqrt(R^2 - dot(p, p)) * r + p;
	float pp = dot(p, p);
	pp = min(pp, R2);
	float sq = sqrt(R2 - pp);
	vec3 x = p + sq * r;

	return x;
}

// Cheap version of computeCubemapVecAccurate
vec3 computeCubemapVecCheap(in vec3 r, in float R2, in vec3 f)
{
	return r;
}

float computeRoughnesSquared(float roughness)
{
	float a2 = roughness * 0.95 + 0.05;
	a2 *= a2 * a2;
	return a2;
}

#endif
