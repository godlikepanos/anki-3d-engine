// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"
#include "shaders/Clusterer.glsl"

#define LIGHT_TEX_BINDING 2
#define LIGHT_UBO_BINDING 0
#define LIGHT_SS_BINDING 0
#define LIGHT_SET 0
#include "shaders/ClusterLightCommon.glsl"

layout(location = 0) in vec2 in_uv;

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_msDepthRt;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2DArray u_noiseTex;

layout(std140, ANKI_UBO_BINDING(0, 3)) uniform ubo0_
{
	vec4 u_linearizeNoiseTexOffsetLayer;
	vec4 u_fogColorFogFactor;
};

#define u_linearize u_linearizeNoiseTexOffsetLayer.xy
#define u_noiseYOffset u_linearizeNoiseTexOffsetLayer.z
#define u_noiseLayer u_linearizeNoiseTexOffsetLayer.w

layout(location = 0) out vec4 out_color;

#define ENABLE_SHADOWS 1
const uint MAX_SAMPLES_PER_CLUSTER = 4u;

// Return the diffuse color without taking into account the diffuse term of the particles.
vec3 computeLightColor(vec3 fragPos[MAX_SAMPLES_PER_CLUSTER], uint iterCount, uint clusterIdx)
{
	vec3 outColor = vec3(0.0);

	uint idxOffset = u_clusters[clusterIdx];

	// Skip decals
	uint count = u_lightIndices[idxOffset];
	idxOffset += count + 1;

	// Point lights
	count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		PointLight light = u_pointLights[u_lightIndices[idxOffset++]];

		for(uint i = 0; i < iterCount; ++i)
		{
			vec3 frag2Light = light.posRadius.xyz - fragPos[i];
			float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

			float shadow = 1.0;
#if ENABLE_SHADOWS
			float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
			if(light.diffuseColorShadowmapId.w >= 0.0)
			{
				shadow = computeShadowFactorOmni(frag2Light,
					shadowmapLayerIdx,
					-1.0 / light.posRadius.w,
					u_lightingUniforms.invViewRotation,
					u_omniMapArr);
			}
#endif

			outColor += light.diffuseColorShadowmapId.rgb * (att * shadow);
		}
	}

	// Spot lights
	count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		SpotLight light = u_spotLights[u_lightIndices[idxOffset++]];

		for(uint i = 0; i < iterCount; ++i)
		{
			vec3 frag2Light = light.posRadius.xyz - fragPos[i];
			float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

			vec3 l = normalize(frag2Light);

			float spot = computeSpotFactor(l, light.outerCosInnerCos.x, light.outerCosInnerCos.y, light.lightDir.xyz);

			float shadow = 1.0;
#if ENABLE_SHADOWS
			float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
			if(shadowmapLayerIdx >= 0.0)
			{
				shadow =
					computeShadowFactorSpot(light.texProjectionMat, fragPos[i], shadowmapLayerIdx, 1, u_spotMapArr);
			}
#endif

			outColor += light.diffuseColorShadowmapId.rgb * (att * spot * shadow);
		}
	}

	return outColor;
}

vec3 fog(in float depth)
{
	float linearDepth = linearizeDepthOptimal(depth, u_linearize.x, u_linearize.y);
	float t = linearDepth * u_fogColorFogFactor.w;
	return u_fogColorFogFactor.rgb * t;
}

void main()
{
	float depth = textureLod(u_msDepthRt, in_uv, 0.0).r;
	float farZ = u_lightingUniforms.projectionParams.z / (u_lightingUniforms.projectionParams.w + depth);

	// Compute the max cluster
	uint maxK = computeClusterKSafe(u_lightingUniforms.nearFarClustererMagicPad1.x,
		u_lightingUniforms.nearFarClustererMagicPad1.y,
		u_lightingUniforms.nearFarClustererMagicPad1.z,
		farZ);
	++maxK;

	vec2 ndc = in_uv * 2.0 - 1.0;

	uint i = uint(in_uv.x * CLUSTER_COUNT.x);
	uint j = uint(in_uv.y * CLUSTER_COUNT.y);

	const float DIST = 1.0 / float(MAX_SAMPLES_PER_CLUSTER);
	vec3 noiseTexUv = vec3(vec2(FB_SIZE) / vec2(NOISE_MAP_SIZE) * in_uv + vec2(0.0, u_noiseYOffset), u_noiseLayer);
	float randFactor = texture(u_noiseTex, noiseTexUv).r;
	float start = DIST * randFactor;
	float factors[MAX_SAMPLES_PER_CLUSTER] = float[](start, DIST + start, 2.0 * DIST + start, 3.0 * DIST + start);

	float kNear = -u_lightingUniforms.nearFarClustererMagicPad1.x;
	vec3 newCol = vec3(0.0);
	uint count = 0;
	for(uint k = 0u; k < maxK; ++k)
	{
		float kFar = computeClusterFar(
			k, u_lightingUniforms.nearFarClustererMagicPad1.x, u_lightingUniforms.nearFarClustererMagicPad1.z);

		vec3 fragPos[MAX_SAMPLES_PER_CLUSTER];

		uint n;
		for(n = 0u; n < MAX_SAMPLES_PER_CLUSTER; ++n)
		{
			float zMedian = mix(kNear, kFar, factors[n]);

			fragPos[n].z = zMedian;
			fragPos[n].xy = ndc * u_lightingUniforms.projectionParams.xy * fragPos[n].z;

			if(zMedian < farZ)
			{
				break;
			}
		}

		uint clusterIdx = k * (CLUSTER_COUNT.x * CLUSTER_COUNT.y) + j * CLUSTER_COUNT.x + i;
		newCol += computeLightColor(fragPos, n, clusterIdx);
		count += n;

		kNear = kFar;
	}

	newCol = newCol * fog(depth) / max(1.0, float(count));
	out_color = vec4(newCol, 1.0 / 3.0);
}
