// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#ifndef ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL
#define ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL

#include "shaders/Functions.glsl"
#include "shaders/Pack.glsl"

const float LIGHT_FRUSTUM_NEAR_PLANE = 0.1 / 4.0;
const uint SHADOW_SAMPLE_COUNT = 16;
const float ESM_CONSTANT = 40.0;

// Fresnel term unreal
vec3 F_Unreal(vec3 specular, float VoH)
{
	return specular + (1.0 - specular) * pow(2.0, (-5.55473 * VoH - 6.98316) * VoH);
}

// D(n,h) aka NDF: GGX Trowbridge-Reitz
float D_GGX(float roughness, float NoH)
{
	float a = roughness * roughness;
	float a2 = a * a;

	float D = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * D * D);
}

// Visibility term: Geometric shadowing divided by BRDF denominator
float V_Schlick(float roughness, float NoV, float NoL)
{
	float k = (roughness * roughness) * 0.5;
	float Vis_SchlickV = NoV * (1.0 - k) + k;
	float Vis_SchlickL = NoL * (1.0 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

vec3 envBRDF(vec3 specular, float roughness, sampler2D integrationLut, float NoV)
{
	float a = roughness * roughness;
	float a2 = a * a;
	vec2 envBRDF = textureLod(integrationLut, vec2(a2, NoV), 0.0).xy;
	return specular * envBRDF.x + min(1.0, 50.0 * specular.g) * envBRDF.y;
}

vec3 diffuseLambert(vec3 diffuse)
{
	return diffuse * (1.0 / PI);
}

// Performs BRDF specular lighting
vec3 computeSpecularColorBrdf(GbufferInfo gbuffer, vec3 viewDir, vec3 frag2Light)
{
	vec3 H = normalize(frag2Light + viewDir);

	float NoL = max(EPSILON, dot(gbuffer.normal, frag2Light));
	float VoH = max(EPSILON, dot(viewDir, H));
	float NoH = max(EPSILON, dot(gbuffer.normal, H));
	float NoV = max(EPSILON, dot(gbuffer.normal, viewDir));

	vec3 F = F_Unreal(gbuffer.specular, VoH);
	float D = D_GGX(gbuffer.roughness, NoH);
	float V = V_Schlick(gbuffer.roughness, NoV, NoL);

	return F * (V * D);
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

	float shadowFactor = textureLod(spotMapArr, texCoords3.xy, 0.0).r;

	return saturate(exp(ESM_CONSTANT * (shadowFactor - linearDepth)));
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

	return saturate(exp(ESM_CONSTANT * (shadowFactor - linearDepth)));
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

float computeAttenuationFactor(float lightRadius, vec3 frag2Light)
{
	float fragLightDist = dot(frag2Light, frag2Light);
	float att = 1.0 - fragLightDist * lightRadius;
	att = max(0.0, att);
	return att * att;
}

#endif
