// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
#include "shaders/IsFsCommon.glsl"
#undef LIGHT_SET
#undef LIGHT_SS_BINDING
#undef LIGHT_TEX_BINDING
#undef LIGHT_UBO_BINDING

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_msRt0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_msRt1;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_msRt2;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_msDepthRt;

layout(location = 0) in vec2 in_texCoord;
layout(location = 1) flat in int in_instanceId;
layout(location = 2) in vec2 in_projectionParams;

layout(location = 0) out vec3 out_color;

const uint TILE_COUNT = TILE_COUNT_X * TILE_COUNT_Y;

// Return frag pos in view space
vec3 getFragPosVSpace()
{
	float depth = textureRt(u_msDepthRt, in_texCoord).r;

	vec3 fragPos;
	fragPos.z = u_lightingUniforms.projectionParams.z / (u_lightingUniforms.projectionParams.w + depth);
	fragPos.xy = in_projectionParams * fragPos.z;

	return fragPos;
}

// Common code for lighting
#define LIGHTING_COMMON_BRDF()                                                                                         \
	vec3 frag2Light = light.posRadius.xyz - fragPos;                                                                   \
	vec3 l = normalize(frag2Light);                                                                                    \
	float nol = max(0.0, dot(normal, l));                                                                              \
	vec3 specC = computeSpecularColorBrdf(viewDir, l, normal, specCol, light.specularColorTexId.rgb, a2, nol);         \
	vec3 diffC = computeDiffuseColor(diffCol, light.diffuseColorShadowmapId.rgb);                                      \
	float att = computeAttenuationFactor(light.posRadius.w, frag2Light);                                               \
	float lambert = nol;

void debugIncorrectColor(inout vec3 c)
{
	if(isnan(c.x) || isnan(c.y) || isnan(c.z) || isinf(c.x) || isinf(c.y) || isinf(c.z))
	{
		c = vec3(1.0, 0.0, 1.0);
	}
}

void readIndirect(in uint idxOffset,
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
	uint count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		ReflectionProbe probe;
		COPY_REFLECTION_PROBE(u_reflectionProbes[u_lightIndices[idxOffset]], probe);
		++idxOffset;

		float R2 = probe.positionRadiusSq.w;
		vec3 center = probe.positionRadiusSq.xyz;

		// Get distance from the center of the probe
		vec3 f = posVSpace - center;

		// Cubemap UV in view space
		vec3 uv = computeCubemapVecAccurate(r, R2, f, u_lightingUniforms.invViewRotation);

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
		uv = computeCubemapVecCheap(n, R2, f, u_lightingUniforms.invViewRotation);
		vec3 id = texture(u_irradianceTex, vec4(uv, cubemapIndex)).rgb;
		diffIndirect = mix(id, diffIndirect, factor);
	}
}

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
	out_color = diffCol * emission;

	// Get counts and offsets
	uint clusterIdx = computeClusterIndexUsingTileIdx(u_lightingUniforms.nearFarClustererMagicPad1.x,
		u_lightingUniforms.nearFarClustererMagicPad1.z,
		fragPos.z,
		in_instanceId,
		TILE_COUNT_X,
		TILE_COUNT_Y);

	uint idxOffset = u_clusters[clusterIdx];
	uint idx;

	// Shadowpass sample count
	uint shadowSampleCount = computeShadowSampleCount(SHADOW_SAMPLE_COUNT, fragPos.z);

	// Point lights
	uint count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		PointLight light;
		COPY_POINT_LIGHT(u_pointLights[u_lightIndices[idxOffset]], light);
		++idxOffset;

		LIGHTING_COMMON_BRDF();

		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(light.diffuseColorShadowmapId.w < 128.0)
		{
			shadow = computeShadowFactorOmni(
				frag2Light, shadowmapLayerIdx, 1.0 / sqrt(light.posRadius.w), u_lightingUniforms.viewMat, u_omniMapArr);

			shadow = dither(shadow, 64.0);
		}

		out_color += (specC + diffC) * (att * max(subsurface, lambert * shadow));
	}

	// Spot lights
	count = u_lightIndices[idxOffset++];
	while(count-- != 0)
	{
		SpotLight light;
		COPY_SPOT_LIGHT(u_spotLights[u_lightIndices[idxOffset]], light);
		++idxOffset;

		LIGHTING_COMMON_BRDF();

		float spot = computeSpotFactor(l, light.outerCosInnerCos.x, light.outerCosInnerCos.y, light.lightDir.xyz);

		float shadow = 1.0;
		float shadowmapLayerIdx = light.diffuseColorShadowmapId.w;
		if(shadowmapLayerIdx < 128.0)
		{
			shadow = computeShadowFactorSpot(
				light.texProjectionMat, fragPos, shadowmapLayerIdx, shadowSampleCount, u_spotMapArr);

			shadow = dither(shadow, 64.0);
		}

		out_color += (diffC + specC) * (att * spot * max(subsurface, lambert * shadow));
	}

#if INDIRECT_ENABLED
	vec3 eye = -viewDir;
	vec3 r = reflect(eye, normal);
	float reflLod = float(IR_MIPMAP_COUNT) * gbuffer.roughness;

	vec3 specIndirect, diffIndirect;
	readIndirect(idxOffset, fragPos, r, normal, reflLod, specIndirect, diffIndirect);

	diffIndirect *= gbuffer.diffuse;

	// Finalize the indirect specular
	float ndotv = dot(gbuffer.normal, viewDir);
	vec2 envBRDF = texture(u_integrationLut, vec2(gbuffer.roughness, ndotv)).xy;
	specIndirect = specIndirect * (gbuffer.specular * envBRDF.x + envBRDF.y);

	out_color += specIndirect + diffIndirect;
#endif

#if 0
	uint count = scount;
	if(count == 0)
	{
		out_color = vec3(1.0, 0.0, 0.0);
	}
	else if(count == 1)
	{
		out_color = vec3(0.0, 1.0, 0.0);
	}
	else if(count == 2)
	{
		out_color = vec3(0.0, 0.0, 1.0);
	}
	else if(count == 3)
	{
		out_color = vec3(1.0, 0.0, 1.0);
	}
	else
	{
		out_color = vec3(1.0, 1.0, 0.0);
	}
#endif
}
