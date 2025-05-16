// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/LightFunctions.hlsl>

struct [raypayload] RtMaterialFetchRayPayload // TODO make it FP16 when you change the GBufferGeneric.ankiprog
{
	HVec3 m_diffuseColor : write(closesthit, miss): read(caller);
	HVec3 m_worldNormal : write(closesthit, miss): read(caller);
	HVec3 m_emission : write(closesthit, miss): read(caller);
	F16 m_textureLod : write(caller): read(closesthit);
	F32 m_rayT : write(closesthit, miss): read(caller);
};

// Have a common resouce interface for all shaders. It should be compatible between all ray shaders in DX and VK
#if ANKI_RAY_GEN_SHADER
#	define SPACE space2

ConstantBuffer<GlobalRendererConstants> g_globalRendererConstants : register(b0, SPACE);

// SRVs
RaytracingAccelerationStructure g_tlas : register(t0, SPACE);
#	if defined(CLIPMAP_VOLUME)
Texture3D<Vec4> g_irradianceVolumes[kIndirectDiffuseClipmapCount] : register(t1, SPACE);
#	else
Texture2D<Vec4> g_gbufferTextures[kIndirectDiffuseClipmapCount] : register(t1, SPACE);
#		define g_depthTex g_gbufferTextures[0]
#		define g_gbufferRt1 g_gbufferTextures[1]
#		define g_gbufferRt2 g_gbufferTextures[2]
#	endif
Texture2D<Vec4> g_envMap : register(t4, SPACE);

#	if defined(CLIPMAP_VOLUME)
StructuredBuffer<U32> g_dummyBuff1 : register(t5, SPACE);
StructuredBuffer<U32> g_dummyBuff2 : register(t6, SPACE);
#	else
StructuredBuffer<GpuSceneGlobalIlluminationProbe> g_giProbes : register(t5, SPACE);
StructuredBuffer<PixelFailedSsr> g_pixelsFailedSsr : register(t6, SPACE);
#	endif

Texture2D<Vec4> g_shadowAtlasTex : register(t7, SPACE);

// UAVs
#	if defined(CLIPMAP_VOLUME)
RWTexture2D<Vec4> g_lightResultTex : register(u0, SPACE);
RWTexture2D<Vec4> g_dummyUav : register(u1, SPACE);
#	else
RWTexture2D<Vec4> g_colorAndPdfTex : register(u0, SPACE);
RWTexture2D<Vec4> g_hitPosAndDepthTex : register(u1, SPACE);
#	endif

// Samplers
SamplerState g_linearClampAnySampler : register(s0, SPACE);
SamplerComparisonState g_shadowSampler : register(s1, SPACE);
SamplerState g_linearRepeatAnySampler : register(s2, SPACE);

template<typename T>
struct GBufferLight
{
	vector<T, 3> m_diffuse;
	vector<T, 3> m_worldNormal;
	vector<T, 3> m_emission;
};

template<typename T>
Bool materialRayTrace(Vec3 rayOrigin, Vec3 rayDir, F32 tMin, F32 tMax, T textureLod, out GBufferLight<T> gbuffer, out F32 rayT, out Bool backfacing,
					  U32 traceFlags = RAY_FLAG_FORCE_OPAQUE)
{
	RtMaterialFetchRayPayload payload;
	payload.m_textureLod = textureLod;
	const U32 sbtRecordOffset = 0u;
	const U32 sbtRecordStride = 0u;
	const U32 missIndex = 0u;
	const U32 cullMask = 0xFFu;
	RayDesc ray;
	ray.Origin = rayOrigin;
	ray.TMin = tMin;
	ray.Direction = rayDir;
	ray.TMax = tMax;
	TraceRay(g_tlas, traceFlags, cullMask, sbtRecordOffset, sbtRecordStride, missIndex, ray, payload);

	rayT = payload.m_rayT;
	const Bool hasHitSky = rayT == kMaxF32;
	backfacing = rayT < 0.0;
	rayT = abs(rayT);
	if(hasHitSky)
	{
		gbuffer = (GBufferLight<T>)0;

		if(g_globalRendererConstants.m_sky.m_type == 0)
		{
			gbuffer.m_emission = g_globalRendererConstants.m_sky.m_solidColor;
		}
		else
		{
			const Vec2 uv = (g_globalRendererConstants.m_sky.m_type == 1) ? equirectangularMapping(rayDir) : octahedronEncode(rayDir);
			gbuffer.m_emission = g_envMap.SampleLevel(g_linearClampAnySampler, uv, 0.0).xyz;
		}
	}
	else
	{
		gbuffer.m_diffuse = payload.m_diffuseColor;
		gbuffer.m_worldNormal = payload.m_worldNormal;
		gbuffer.m_emission = payload.m_emission;
	}

	return !hasHitSky;
}

template<typename T>
vector<T, 3> directLighting(GBufferLight<T> gbuffer, Vec3 hitPos, Bool isSky, Bool tryShadowmapFirst, F32 shadowTMax,
							U32 traceFlags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)
{
	vector<T, 3> color = gbuffer.m_emission;

	if(!isSky)
	{
		const DirectionalLight dirLight = g_globalRendererConstants.m_directionalLight;

		// Trace shadow
		Vec4 vv4 = mul(g_globalRendererConstants.m_matrices.m_viewProjection, Vec4(hitPos, 1.0));
		vv4.xy /= vv4.w;
		const Bool bInsideFrustum = all(vv4.xy > -1.0) && all(vv4.xy < 1.0) && vv4.w > 0.0;

		F32 shadow;
		if(bInsideFrustum && tryShadowmapFirst)
		{
			const F32 negativeZViewSpace = -mul(g_globalRendererConstants.m_matrices.m_view, Vec4(hitPos, 1.0)).z;
			const U32 shadowCascadeCount = dirLight.m_shadowCascadeCount_31bit_active_1bit >> 1u;

			const U32 cascadeIdx = computeShadowCascadeIndex(negativeZViewSpace, dirLight.m_shadowCascadeDistances, shadowCascadeCount);

			shadow = computeShadowFactorDirLight<F32>(dirLight, cascadeIdx, hitPos, g_shadowAtlasTex, g_shadowSampler);
		}
		else
		{
			RayQuery<RAY_FLAG_NONE> q;
			const U32 cullMask = 0xFFu;
			RayDesc ray;
			ray.Origin = hitPos;
			ray.TMin = 0.01;
			ray.Direction = -dirLight.m_direction;
			ray.TMax = shadowTMax;
			q.TraceRayInline(g_tlas, traceFlags, cullMask, ray);
			q.Proceed();
			shadow = (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) ? 0.0 : 1.0;
		}

		// Do simple light shading

		const vector<T, 3> l = -dirLight.m_direction;
		const T lambert = max(T(0), dot(l, gbuffer.m_worldNormal));
		const vector<T, 3> diffC = diffuseLobe(gbuffer.m_diffuse);
		color += diffC * dirLight.m_diffuseColor * lambert * shadow;
	}

	return color;
}
#endif // ANKI_RAY_GEN_SHADER
