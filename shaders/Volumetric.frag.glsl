// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_prevResultTex;

layout(std140, ANKI_UBO_BINDING(0, 3)) uniform ubo0_
{
	vec4 u_linearizePad2;
	vec4 u_fogColorFogFactor;
};

layout(location = 0) out vec3 out_color;

#define ENABLE_SHADOWS 0

vec3 computeLightColor(vec3 diffCol, vec3 fragPos, uint clusterIdx)
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

		vec3 diffC = computeDiffuseColor(diffCol, light.diffuseColorShadowmapId.rgb);

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		float shadow = 1.0;
#if ENABLE_SHADOWS
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w < 128.0)
		{
			shadow = computeShadowFactorOmni(
				frag2Light, shadowmapLayerIdx, -1.0 / light.posRadius.w, u_lightingUniforms.viewMat, u_omniMapArr);
		}
#endif

		outColor += diffC * (att * shadow);
	}

	// Spot lights
	count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		SpotLight light = u_spotLights[u_lightIndices[idxOffset++]];

		vec3 diffC = computeDiffuseColor(diffCol, light.diffuseColorShadowmapId.rgb);

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		vec3 l = normalize(frag2Light);

		float spot = computeSpotFactor(l, light.outerCosInnerCos.x, light.outerCosInnerCos.y, light.lightDir.xyz);

		float shadow = 1.0;
#if ENABLE_SHADOWS
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(shadowmapLayerIdx < 128.0)
		{
			shadow = computeShadowFactorSpot(light.texProjectionMat, fragPos, shadowmapLayerIdx, 1, u_spotMapArr);
		}
#endif

		outColor += diffC * (att * spot * shadow);
	}

	return outColor;
}

vec3 fog(in float depth)
{
	float linearDepth = linearizeDepthOptimal(depth, u_linearizePad2.x, u_linearizePad2.y);
	float t = linearDepth * u_fogColorFogFactor.w;
	return u_fogColorFogFactor.rgb * t;
}

void main()
{
	float depth = textureLod(u_msDepthRt, in_uv, 0.0).r;
	float farZ = u_lightingUniforms.projectionParams.z / (u_lightingUniforms.projectionParams.w + depth);

	vec3 diffCol = fog(depth);
	vec3 prevCol = texture(u_prevResultTex, in_uv).rgb;

	vec2 ndc = in_uv * 2.0 - 1.0;

	uint i = uint(gl_FragCoord.x * 4.0) >> 6;
	uint j = uint(gl_FragCoord.y * 4.0) >> 6;

	float randFactor = rand(ndc + u_lightingUniforms.rendererSizeTimePad1.z);
	// float randFactor = rand(in_uv);

	float kNear = -u_lightingUniforms.nearFarClustererMagicPad1.x;
	uint k;
	vec3 newCol = vec3(0.0);
	for(k = 0u; k < CLUSTER_COUNT.z; ++k)
	{
		float kFar = computeClusterFar(
			k, u_lightingUniforms.nearFarClustererMagicPad1.x, u_lightingUniforms.nearFarClustererMagicPad1.z);

		float zMedian = mix(kNear, kFar, randFactor);
		if(zMedian < farZ)
		{
			break;
		}

		uint clusterIdx = k * (CLUSTER_COUNT.x * CLUSTER_COUNT.y) + j * CLUSTER_COUNT.x + i;

		vec3 fragPos;
		fragPos.z = zMedian;
		fragPos.xy = ndc * u_lightingUniforms.projectionParams.xy * fragPos.z;

		newCol += computeLightColor(diffCol, fragPos, clusterIdx);

		kNear = kFar;
	}

	newCol = newCol / max(1.0, float(k));

	out_color = mix(newCol, prevCol, 0.666);
	// out_color = newCol;
}
