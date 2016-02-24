// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Pack.glsl"
#include "shaders/Clusterer.glsl"
#include "shaders/Functions.glsl"

#define LIGHT_SET 0
#define LIGHT_SS_BINDING 0
#define LIGHT_UBO_BINDING 0
#define LIGHT_TEX_BINDING 4
#include "shaders/LightResources.glsl"
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING
#undef LIGHT_UBO_BINDING

layout(binding = 0) uniform sampler2D u_msRt0;
layout(binding = 1) uniform sampler2D u_msRt1;
layout(binding = 2) uniform sampler2D u_msRt2;
layout(binding = 3) uniform sampler2D u_msDepthRt;

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) flat in int in_instanceId;
layout(location = 2) in vec2 in_projectionParams;

layout(location = 0) out vec3 out_color;

#include "shaders/LightFunctions.glsl"

const uint TILE_COUNT = TILE_COUNT_X * TILE_COUNT_Y;

//==============================================================================
// Return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = textureRt(u_msDepthRt, in_texCoord).r;

	vec3 fragPos;
	fragPos.z = u_lightingUniforms.projectionParams.z
		/ (u_lightingUniforms.projectionParams.w + depth);
	fragPos.xy = in_projectionParams * fragPos.z;

	return fragPos;
}

//==============================================================================
// Common code for lighting

#define LIGHTING_COMMON_BRDF()                                                 \
	vec3 frag2Light = light.posRadius.xyz - fragPos;                           \
	vec3 l = normalize(frag2Light);                                            \
	float nol = max(0.0, dot(normal, l));                                      \
	vec3 specC = computeSpecularColorBrdf(                                     \
		viewDir, l, normal, specCol, light.specularColorTexId.rgb, a2, nol);   \
	vec3 diffC =                                                               \
		computeDiffuseColor(diffCol, light.diffuseColorShadowmapId.rgb);       \
	float att = computeAttenuationFactor(light.posRadius.w, frag2Light);       \
	float lambert = nol;

//==============================================================================
void debugIncorrectColor(inout vec3 c)
{
	if(isnan(c.x) || isnan(c.y) || isnan(c.z) || isinf(c.x) || isinf(c.y)
		|| isinf(c.z))
	{
		c = vec3(1.0, 0.0, 1.0);
	}
}

//==============================================================================
void main()
{
	// Get frag pos in view space
	vec3 fragPos = getFragPosVSpace();
	vec3 viewDir = normalize(-fragPos);

	// Decode GBuffer
	vec3 normal;
	vec3 diffCol;
	vec3 specCol;
	float roughness;
	float subsurface;
	float emission;
	float metallic;

	GbufferInfo gbuffer;
	readGBuffer(u_msRt0, u_msRt1, u_msRt2, in_texCoord, 0.0, gbuffer);
	diffCol = gbuffer.diffuse;
	specCol = gbuffer.specular;
	normal = gbuffer.normal;
	roughness = gbuffer.roughness;
	metallic = gbuffer.metallic;
	subsurface = gbuffer.subsurface;
	emission = gbuffer.emission;

	float a2 = pow(roughness, 2.0);

	// Ambient and emissive color
	out_color =
		diffCol * u_lightingUniforms.sceneAmbientColor.rgb + diffCol * emission;

	// Get counts and offsets
	uint clusterIdx = computeClusterIndexUsingTileIdx(
		u_lightingUniforms.nearFarClustererMagicPad1.x,
		u_lightingUniforms.nearFarClustererMagicPad1.z,
		fragPos.z,
		in_instanceId,
		TILE_COUNT_X,
		TILE_COUNT_Y);

	uint lightOffset;
	uint pointLightCount;
	uint spotLightCount;
	uint probeCount;
	getClusterInfo(
		clusterIdx, lightOffset, pointLightCount, spotLightCount, probeCount);

	// Shadowpass sample count
	uint shadowSampleCount =
		computeShadowSampleCount(SHADOW_SAMPLE_COUNT, fragPos.z);

	// Point lights
	for(uint i = 0U; i < pointLightCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
		PointLight light = u_pointLights[lightId];

		LIGHTING_COMMON_BRDF();

		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w < 128.0)
		{
			shadow = computeShadowFactorOmni(
				frag2Light, shadowmapLayerIdx, 1.0 / sqrt(light.posRadius.w));

			shadow = dither(shadow, 64.0);
		}

		out_color +=
			(specC + diffC) * (att * max(subsurface, lambert * shadow));
	}

	// Spot lights
	for(uint i = 0U; i < spotLightCount; ++i)
	{
		uint lightId = u_lightIndices[lightOffset++];
		SpotLight light = u_spotLights[lightId];

		LIGHTING_COMMON_BRDF();

		float spot = computeSpotFactor(l,
			light.outerCosInnerCos.x,
			light.outerCosInnerCos.y,
			light.lightDir.xyz);

		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(shadowmapLayerIdx < 128.0)
		{
			shadow = computeShadowFactorSpot(light.texProjectionMat,
				fragPos,
				shadowmapLayerIdx,
				shadowSampleCount);

			shadow = dither(shadow, 64.0);
		}

		out_color +=
			(diffC + specC) * (att * spot * max(subsurface, lambert * shadow));
	}

#if INDIRECT_ENABLED
	vec3 eye = -viewDir;
	vec3 r = reflect(eye, normal);
	float reflLod = float(IR_MIPMAP_COUNT) * gbuffer.roughness;

	vec3 specIndirect, diffIndirect;
	readIndirect(lightOffset,
		probeCount,
		fragPos,
		r,
		normal,
		reflLod,
		specIndirect,
		diffIndirect);

	diffIndirect *= gbuffer.diffuse;

	// Finalize the indirect specular
	float ndotv = dot(gbuffer.normal, viewDir);
	vec2 envBRDF = texture(u_integrationLut, vec2(gbuffer.roughness, ndotv)).xy;
	specIndirect = specIndirect * (gbuffer.specular * envBRDF.x + envBRDF.y);

	out_color += specIndirect + diffIndirect;
#endif

// out_color = diffCol;
#if 0
	if(pointLightCount == 0)
	{
	}
	else if(pointLightCount == 1)
	{
		out_color += vec3(0.0, 1.0, 0.0);
	}
	else if(pointLightCount == 2)
	{
		out_color += vec3(0.0, 0.0, 1.0);
	}
	else if(pointLightCount == 3)
	{
		out_color += vec3(1.0, 0.0, 1.0);
	}
	else
	{
		out_color += vec3(1.0, 0.0, 0.0);
	}
#endif
}
