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

#define anki_u_time u_lightingUniforms.rendererSizeTimePad1.z
#define RENDERER_SIZE (u_lightingUniforms.rendererSizeTimePad1.xy * 0.5)

layout(location = 0) in vec3 in_vertPosViewSpace;
layout(location = 1) flat in float in_alpha;

layout(location = 0) out vec4 out_color;

#if PASS == COLOR
#define texture_DEFINED
#endif

#define getAlpha_DEFINED
float getAlpha()
{
	return in_alpha;
}

#define getPointCoord_DEFINED
#define getPointCoord() gl_PointCoord

#if PASS == COLOR
#define writeGBuffer_DEFINED
void writeGBuffer(in vec4 color)
{
	out_color = vec4(color.rgb, 1.0 - color.a);
}
#endif

#if PASS == COLOR
#define particleAlpha_DEFINED
void particleAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define particleSoftTextureAlpha_DEFINED
void particleSoftTextureAlpha(in sampler2D depthMap, in sampler2D tex, in float alpha)
{
	vec2 screenSize = 1.0 / RENDERER_SIZE;

	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 50.0, 0.0, 1.0);

	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	// color.a *= softalpha;

	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define particleTextureAlpha_DEFINED
void particleTextureAlpha(in sampler2D tex, vec4 mulColor, vec4 addColor, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord) * mulColor + addColor;
	color.a *= alpha;

	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define particleSoftColorAlpha_DEFINED
void particleSoftColorAlpha(in sampler2D depthMap, in vec3 icolor, in float alpha)
{
	vec2 screenSize = 1.0 / RENDERER_SIZE;

	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 50.0, 0.0, 1.0);

	vec2 pix = (1.0 - abs(gl_PointCoord * 2.0 - 1.0));
	float roundFactor = pix.x * pix.y;

	vec4 color;
	color.rgb = icolor;
	color.a = alpha * softalpha * roundFactor;
	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define computeLightColor_DEFINED
vec3 computeLightColor(vec3 diffCol)
{
	vec3 outColor = vec3(0.0);

	// Compute frag pos in view space
	vec3 fragPos;
	{
		float depth = gl_FragCoord.z;
		fragPos.z = u_lightingUniforms.projectionParams.z / (u_lightingUniforms.projectionParams.w + depth);

		vec2 screenSize = 1.0 / RENDERER_SIZE;

		vec2 ndc = gl_FragCoord.xy * screenSize * 2.0 - 1.0;

		fragPos.xy = ndc * u_lightingUniforms.projectionParams.xy * fragPos.z;
	}

	// Find the cluster and then the light counts
	uint clusterIdx = computeClusterIndex(gl_FragCoord.xy / RENDERER_SIZE,
		u_lightingUniforms.nearFarClustererMagicPad1.x,
		u_lightingUniforms.nearFarClustererMagicPad1.z,
		fragPos.z,
		u_lightingUniforms.tileCount.x,
		u_lightingUniforms.tileCount.y);

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
			shadow = computeShadowFactorOmni(frag2Light,
				shadowmapLayerIdx,
				light.specularColorRadius.w,
				u_lightingUniforms.invViewRotation,
				u_omniMapArr);
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
#endif

#if PASS == COLOR
#define particleTextureAlphaLight_DEFINED
void particleTextureAlphaLight(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;

	color.rgb = computeLightColor(color.rgb);

	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define particleAnimatedTextureAlphaLight_DEFINED
void particleAnimatedTextureAlphaLight(sampler2DArray tex, float alpha, float layerCount, float period)
{
	vec4 color = readAnimatedTextureRgba(tex, layerCount, period, gl_PointCoord, anki_u_time);

	color.rgb = computeLightColor(color.rgb);

	color.a *= alpha;
	writeGBuffer(color);
}
#endif

#if PASS == COLOR
#define fog_DEFINED
void fog(in sampler2D depthMap, in vec3 color, in float fogScale)
{
	const vec2 screenSize = 1.0 / RENDERER_SIZE;

	vec2 texCoords = gl_FragCoord.xy * screenSize;
	float depth = texture(depthMap, texCoords).r;
	float diff;

	if(depth < 1.0)
	{
		float zNear = u_lightingUniforms.nearFarClustererMagicPad1.x;
		float zFar = u_lightingUniforms.nearFarClustererMagicPad1.y;
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

#endif
