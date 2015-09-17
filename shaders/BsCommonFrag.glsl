// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common code for all fragment shaders of BS
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

// Global resources
layout(TEX_BINDING(1, 0)) uniform sampler2D anki_u_msDepthRt;
#define LIGHT_SET 1
#define LIGHT_SS_BINDING 0
#define LIGHT_TEX_BINDING 1
#pragma anki include "shaders/LightResources.glsl"
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING

layout(location = 0) in vec3 in_vertPosViewSpace;
layout(location = 1) flat in float in_alpha;

layout(location = 0) out vec4 out_color;

#pragma anki include "shaders/LightFunctions.glsl"

//==============================================================================
#if PASS == COLOR
#	define texture_DEFINED
#endif

//==============================================================================
#define getAlpha_DEFINED
float getAlpha()
{
	return in_alpha;
}

//==============================================================================
#define getPointCoord_DEFINED
#define getPointCoord() gl_PointCoord

//==============================================================================
#if PASS == COLOR
#	define writeGBuffer_DEFINED
void writeGBuffer(in vec4 color)
{
	out_color = color;
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleAlpha_DEFINED
void particleAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleSoftTextureAlpha_DEFINED
void particleSoftTextureAlpha(in sampler2D depthMap, in sampler2D tex,
	in float alpha)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH),
		1.0 / float(ANKI_RENDERER_HEIGHT));

	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 50.0, 0.0, 1.0);

	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	//color.a *= softalpha;

	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleTextureAlpha_DEFINED
void particleTextureAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;

	writeGBuffer(color);
}
#endif

//==============================================================================
#if PASS == COLOR
#	define particleSoftColorAlpha_DEFINED
void particleSoftColorAlpha(in sampler2D depthMap, in vec3 icolor,
	in float alpha)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH),
		1.0 / float(ANKI_RENDERER_HEIGHT));

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

//==============================================================================
#if PASS == COLOR
#	define fog_DEFINED
void fog(in sampler2D depthMap, in vec3 color, in float fogScale)
{
	const vec2 screenSize = vec2(
		1.0 / float(ANKI_RENDERER_WIDTH),
		1.0 / float(ANKI_RENDERER_HEIGHT));

	vec2 texCoords = gl_FragCoord.xy * screenSize;
	float depth = texture(depthMap, texCoords).r;
	float zNear = u_nearFarClustererDivisor.x;
	float zFar = u_nearFarClustererDivisor.y;
	float linearDepth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));

	float depth2 = gl_FragCoord.z;
	float linearDepth2 =
		(2.0 * zNear) / (zFar + zNear - depth2 * (zFar - zNear));

	float diff = linearDepth - linearDepth2;

	//writeGBuffer(vec4(vec3(diff * fogScale), 1.0));
	writeGBuffer(vec4(color, diff * fogScale));
}
#endif

//==============================================================================
#if PASS == COLOR
#	define computeLightColor_DEFINED
vec3 computeLightColor(vec3 diffColor)
{
	vec3 fragPos = in_vertPosViewSpace;
	vec3 outColor = diffCol * u_sceneAmbientColor.rgb;

	// Find the cluster and then the light counts
	uint lightOffset;
	uint pointLightsCount;
	uint spotLightsCount;
	{
		uint k = calcClusterSplit(fragPos.z);

		vec2 tilef = ceil(gl_FragCoord.xy / u_tileCount.xy);
		uint tile = uint(tilef.y) * u_tileCount.x + uint(tilef.x);

		uint cluster = u_clusters[tile + k * u_tileCount.z];

		lightOffset = cluster >> 16u;
		pointLightsCount = (cluster >> 8u) & 0xFFu;
		spotLightsCount = cluster & 0xFFu;
	}

	// Point lights
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
		PointLight light = u_pointLights[lightId];

		vec3 diffC = computeDiffuseColor(
			diffCol, light.diffuseColorShadowmapId.rgb);

		vec3 frag2Light = light.posRadius.xyz - fragPos;
		float att = computeAttenuationFactor(light.posRadius.w, frag2Light);

		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w < 128.0)
		{
			shadow = computeShadowFactorOmni(frag2Light,
				shadowmapLayerIdx, -1.0 / light.posRadius.w);
		}

		outColor += diffC * (att * shadow);
	}

	return outColor;
}
#endif
