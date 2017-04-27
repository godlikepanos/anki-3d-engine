// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FS_COMMON_FRAG_GLSL
#define ANKI_SHADERS_FS_COMMON_FRAG_GLSL

// Common code for all fragment shaders of BS
#include "shaders/Common.glsl"
#include "shaders/MsFsCommon.glsl"
#include "shaders/Functions.glsl"
#include "shaders/Clusterer.glsl"

// Global resources
layout(ANKI_TEX_BINDING(1, 0)) uniform sampler2D anki_msDepthRt;
#define LIGHT_SET 1
#define LIGHT_UBO_BINDING 0
#define LIGHT_SS_BINDING 0
#define LIGHT_TEX_BINDING 1
#include "shaders/ClusterLightCommon.glsl"

#define anki_u_time u_time
#define RENDERER_SIZE (u_lightingUniforms.rendererSizeTimePad1.xy * 0.5)

layout(location = 0) flat in float in_alpha;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_posViewSpace;

layout(location = 0) out vec4 out_color;

void writeGBuffer(in vec4 color)
{
	out_color = vec4(color.rgb, 1.0 - color.a);
}

vec4 readAnimatedTextureRgba(sampler2DArray tex, float period, vec2 uv, float time)
{
	float layerCount = float(textureSize(tex, 0).z);
	float layer = mod(time * layerCount / period, layerCount);
	return texture(tex, vec3(uv, layer));
}

vec3 computeLightColor(vec3 diffCol)
{
	vec3 outColor = vec3(0.0);

	// Compute frag pos in view space
	vec3 fragPos = in_posViewSpace;

	// Find the cluster and then the light counts
	uint clusterIdx = computeClusterIndex(
		gl_FragCoord.xy / RENDERER_SIZE, u_near, u_clustererMagic, fragPos.z, u_clusterCountX, u_clusterCountY);

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

#if LOD > 1
		const float shadow = 1.0;
#else
		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w >= 0.0)
		{
			shadow = computeShadowFactorOmni(
				frag2Light, shadowmapLayerIdx, light.specularColorRadius.w, u_invViewRotation, u_omniMapArr);
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

#if LOD > 1
		const float shadow = 1.0;
#else
		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(shadowmapLayerIdx >= 0.0)
		{
			shadow = computeShadowFactorSpot(light.texProjectionMat, fragPos, shadowmapLayerIdx, 1, u_spotMapArr);
		}
#endif

		outColor += diffC * (att * spot * shadow);
	}

	return outColor;
}

void particleAlpha(vec4 color, vec4 scaleColor, vec4 biasColor)
{
	writeGBuffer(color * mulColor + addColor);
}

void fog(in sampler2D depthMap, in vec3 color, in float fogScale)
{
	const vec2 screenSize = 1.0 / RENDERER_SIZE;

	vec2 texCoords = gl_FragCoord.xy * screenSize;
	float depth = texture(depthMap, texCoords).r;
	float diff;

	if(depth < 1.0)
	{
		float zNear = u_near;
		float zFar = u_far;
		vec2 linearDepths = (2.0 * zNear) / (zFar + zNear - vec2(depth, gl_FragCoord.z) * (zFar - zNear));

		diff = linearDepths.x - linearDepths.y;
	}
	else
	{
		// The depth buffer is cleared at this place. Set the diff to zero to avoid weird pop ups
		diff = 0.0;
	}

	writeGBuffer(vec4(color, diff * fogScale));
}

#endif
