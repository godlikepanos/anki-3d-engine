// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"
#include "shaders/Clusterer.glsl"

#define LIGHT_TEX_BINDING 3
#define LIGHT_UBO_BINDING 0
#define LIGHT_SS_BINDING 0
#define LIGHT_SET 0
#define LIGHT_LIGHTS
#include "shaders/ClusterLightCommon.glsl"

layout(location = 0) in vec2 in_uv;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_msDepthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2DArray u_noiseTex;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_historyRt;

layout(std140, ANKI_UBO_BINDING(0, 3), row_major) uniform ubo0_
{
	vec4 u_linearizeNoiseTexOffsetLayer;
	vec4 u_fogParticleColorPad1;
	mat4 u_prevViewProjMatMulInvViewProjMat;
};

#define u_linearize readFirstInvocationARB(u_linearizeNoiseTexOffsetLayer.xy)
#define u_noiseYOffset readFirstInvocationARB(u_linearizeNoiseTexOffsetLayer.z)
#define u_noiseLayer readFirstInvocationARB(u_linearizeNoiseTexOffsetLayer.w)
#define u_fogParticleColor readFirstInvocationARB(u_fogParticleColorPad1.rgb)

layout(location = 0) out vec3 out_color;

#define ENABLE_SHADOWS 1
const uint MAX_SAMPLES_PER_CLUSTER = 4u;
const float DIST_BETWEEN_SAMPLES = 0.25;

// Return the diffuse color without taking into account the diffuse term of the particles.
vec3 computeLightColor(vec3 fragPos, uint plightCount, uint plightIdx, uint slightCount, uint slightIdx)
{
	vec3 outColor = vec3(0.0);

	// Point lights
	while(plightCount-- != 0)
	{
		PointLight light = u_pointLights[u_lightIndices[plightIdx++]];
		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float factor = computeAttenuationFactor(light.posRadius.w, frag2Light);

#if ENABLE_SHADOWS
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w >= 0.0)
		{
			factor *= computeShadowFactorOmni(
				frag2Light, shadowmapLayerIdx, -1.0 / light.posRadius.w, u_invViewRotation, u_omniMapArr);
		}
#endif

		outColor += light.diffuseColorShadowmapId.rgb * factor;
	}

	// Spot lights
	while(slightCount-- != 0)
	{
		SpotLight light = u_spotLights[u_lightIndices[slightIdx++]];
		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float factor = computeAttenuationFactor(light.posRadius.w, frag2Light);

		vec3 l = normalize(frag2Light);

		factor *= computeSpotFactor(l, light.outerCosInnerCos.x, light.outerCosInnerCos.y, light.lightDir.xyz);

#if ENABLE_SHADOWS
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(shadowmapLayerIdx >= 0.0)
		{
			factor *= computeShadowFactorSpot(light.texProjectionMat, fragPos, shadowmapLayerIdx, 1, u_spotMapArr);
		}
#endif

		outColor += light.diffuseColorShadowmapId.rgb * factor;
	}

	return outColor;
}

void main()
{
	float depth = textureLod(u_msDepthRt, in_uv, 0.0).r;

	vec3 ndc = UV_TO_NDC(vec3(in_uv, depth));

	vec4 v4 = u_prevViewProjMatMulInvViewProjMat * vec4(ndc, 1.0);
	vec2 oldUv = NDC_TO_UV(v4.xy / v4.w);
	vec3 history = textureLod(u_historyRt, oldUv, 0.0).rgb;

	vec3 farPos;
	farPos.z = u_unprojectionParams.z / (u_unprojectionParams.w + depth);
	farPos.xy = ndc.xy * u_unprojectionParams.xy * farPos.z;
	vec3 viewDir = normalize(farPos);

	uint i = uint(in_uv.x * float(CLUSTER_COUNT.x));
	uint j = uint(in_uv.y * float(CLUSTER_COUNT.y));
	uint ij = j * CLUSTER_COUNT.x + i;

	vec3 noiseTexUv = vec3(vec2(FB_SIZE) / vec2(NOISE_MAP_SIZE) * in_uv + vec2(0.0, u_noiseYOffset), u_noiseLayer);
	float randFactor = clamp(texture(u_noiseTex, noiseTexUv).r, EPSILON, 1.0 - EPSILON);

	float kNear = -u_near;
	vec3 newCol = vec3(0.0);
	for(uint k = 0u; k < CLUSTER_COUNT.z; ++k)
	{
		float kFar = computeClusterFar(k, u_near, u_clustererMagic);

		//
		// Compute sample count
		//
		float diff = kNear - kFar;
		float samplesf = clamp(diff / DIST_BETWEEN_SAMPLES, 1.0, float(MAX_SAMPLES_PER_CLUSTER));
		float dist = 1.0 / samplesf;
		float start = dist * randFactor;

		//
		// Find index ranges
		//
		uint clusterIdx = k * (CLUSTER_COUNT.x * CLUSTER_COUNT.y) + ij;
		uint idxOffset = u_clusters[clusterIdx];

		// Skip decals
		uint count = u_lightIndices[idxOffset];
		idxOffset += count + 1;

		uint plightCount = u_lightIndices[idxOffset++];
		uint plightIdx = idxOffset;
		idxOffset += plightCount;

		uint slightCount = u_lightIndices[idxOffset++];
		uint slightIdx = idxOffset;

		for(float factor = start; factor <= 1.0; factor += dist)
		{
			float zMedian = mix(kNear, kFar, factor);

			if(zMedian < farPos.z)
			{
				k = CLUSTER_COUNT.z; // Break the outer loop
				break;
			}

			vec3 fragPos = viewDir * (zMedian / viewDir.z);

			newCol += computeLightColor(fragPos, plightCount, plightIdx, slightCount, slightIdx);
		}

		kNear = kFar;
	}

	newCol *= u_fogParticleColor;

	history = max(history, newCol);
	out_color = mix(history, newCol, 1.0 / 16.0);
}
