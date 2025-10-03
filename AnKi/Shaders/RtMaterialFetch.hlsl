// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Include/GpuSceneTypes.h>
#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/LightFunctions.hlsl>
#include <AnKi/Shaders/Sky.hlsl>

struct [raypayload] RtMaterialFetchRayPayload
{
	// Use FP32 on D3D because FP16 crashes at least on nVidia
#if ANKI_GR_BACKEND_VULKAN
#	define PAYLOAD_SCALAR F16
#else
#	define PAYLOAD_SCALAR F32
#endif
	vector<PAYLOAD_SCALAR, 3> m_diffuseColor : write(closesthit, miss): read(caller);
	vector<PAYLOAD_SCALAR, 3> m_worldNormal : write(closesthit, miss): read(caller);
	vector<PAYLOAD_SCALAR, 3> m_emission : write(closesthit, miss): read(caller);
	PAYLOAD_SCALAR m_textureLod : write(caller): read(closesthit);
	F32 m_rayT : write(closesthit, miss): read(caller);
#undef PAYLOAD_SCALAR
};

// Have a common resouce interface for all shaders. It should be compatible between all ray shaders in DX and VK
#if ANKI_RAY_GEN_SHADER || defined(INCLUDE_ALL)
#	define SPACE space2

ConstantBuffer<GlobalRendererConstants> g_globalRendererConstants : register(b0, SPACE);

// SRVs
RaytracingAccelerationStructure g_tlas : register(t0, SPACE);

Texture2D<Vec4> g_envMap : register(t1, SPACE);
Texture2D<Vec4> g_shadowAtlasTex : register(t2, SPACE);

#	if !defined(CLIPMAP_VOLUME)
StructuredBuffer<GpuSceneGlobalIlluminationProbe> g_giProbes : register(t3, SPACE);

Texture2D<Vec4> g_depthTex : register(t4, SPACE);
Texture2D<Vec4> g_gbufferRt1 : register(t5, SPACE);
Texture2D<Vec4> g_gbufferRt2 : register(t6, SPACE);

StructuredBuffer<PixelFailedSsr> g_pixelsFailedSsr : register(t7, SPACE);
#	endif

// Output of GpuVisibilityLocalLights:
StructuredBuffer<GpuSceneLight> g_lights : register(t8, SPACE);
StructuredBuffer<U32> g_lightIndexCountsPerCell : register(t9, SPACE);
StructuredBuffer<U32> g_lightIndexOffsetsPerCell : register(t10, SPACE);
StructuredBuffer<U32> g_lightIndexList : register(t11, SPACE);

// UAVs
#	if defined(CLIPMAP_VOLUME)
RWTexture2D<Vec4> g_lightResultTex : register(u0, SPACE);
RWTexture2D<Vec4> g_dummyUav : register(u1, SPACE);
#	else
RWTexture2D<Vec4> g_colorAndPdfTex : register(u0, SPACE);
RWTexture2D<Vec4> g_hitPosAndDepthTex : register(u1, SPACE);
#	endif

// Samplers
SamplerState g_linearAnyClampSampler : register(s0, SPACE);
SamplerComparisonState g_shadowSampler : register(s1, SPACE);
SamplerState g_linearAnyRepeatSampler : register(s2, SPACE);

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
		gbuffer.m_emission = sampleSkyCheap<T>(g_globalRendererConstants.m_sky, rayDir, g_linearAnyClampSampler);
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
Bool materialRayTraceInlineRt(Vec3 rayOrigin, Vec3 rayDir, F32 tMin, F32 tMax, T textureLod, out GBufferLight<T> gbuffer, out F32 rayT,
							  out Bool backfacing)
{
	gbuffer = (GBufferLight<T>)0;
	Bool hit = false;
	rayT = -1.0;
	backfacing = false;

#	if ANKI_GR_BACKEND_VULKAN
	RayQuery<RAY_FLAG_FORCE_OPAQUE> q;
	const U32 cullMask = 0xFFu;
	RayDesc ray;
	ray.Origin = rayOrigin;
	ray.TMin = tMin;
	ray.Direction = rayDir;
	ray.TMax = tMax;
	q.TraceRayInline(g_tlas, RAY_FLAG_FORCE_OPAQUE, cullMask, ray);
	while(q.Proceed())
	{
	}
	hit = q.CommittedStatus() == COMMITTED_TRIANGLE_HIT;

	if(!hit)
	{
		backfacing = false;
		gbuffer.m_emission = sampleSkyCheap<T>(g_globalRendererConstants.m_sky, rayDir, g_linearAnyClampSampler);
		rayT = -1.0;
	}
	else
	{
		backfacing = q.CommittedTriangleFrontFace();

		// Read the diff color from the AS instance
		UVec3 diffColoru = q.CommittedInstanceID();
		diffColoru >>= UVec3(16, 8, 0);
		diffColoru &= 0xFF;
		gbuffer.m_diffuse = Vec3(diffColoru) / 255.0;

		// Compute the normal
		const Vec3 positions[3] = spvRayQueryGetIntersectionTriangleVertexPositionsKHR(q, SpvRayQueryCommittedIntersectionKHR);
		const Vec3 vertNormal = normalize(cross(positions[1] - positions[0], positions[2] - positions[1]));
		gbuffer.m_worldNormal = normalize(mul(q.CommittedObjectToWorld3x4(), Vec4(vertNormal, 0.0)));

		rayT = q.CommittedRayT();
	}
#	endif

	return hit;
}

Bool rayVisibility(Vec3 rayOrigin, Vec3 rayDir, F32 tMax, U32 traceFlags)
{
	RayQuery<RAY_FLAG_NONE> q;
	const U32 cullMask = 0xFFu;
	RayDesc ray;
	ray.Origin = rayOrigin;
	ray.TMin = 0.01;
	ray.Direction = rayDir;
	ray.TMax = tMax;
	q.TraceRayInline(g_tlas, traceFlags, cullMask, ray);
	while(q.Proceed())
	{
	}
	const Bool hit = q.CommittedStatus() == COMMITTED_TRIANGLE_HIT;

	return hit;
}

template<typename T>
vector<T, 3> directLighting(GBufferLight<T> gbuffer, Vec3 hitPos, Bool isSky, Bool tryShadowmapFirst, F32 shadowTMax, Bool doLocalLightShadow,
							U32 traceFlags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH)
{
	vector<T, 3> color = gbuffer.m_emission;

	if(isSky)
	{
		return color;
	}

	// Sun
	const DirectionalLight dirLight = g_globalRendererConstants.m_directionalLight;
	if(dirLight.m_active)
	{
		F32 shadow = 1.0;
		if(dirLight.m_shadowCascadeCount)
		{
			// Trace shadow
			Vec4 vv4 = mul(g_globalRendererConstants.m_matrices.m_viewProjection, Vec4(hitPos, 1.0));
			vv4.xy /= vv4.w;
			const Bool bInsideFrustum = all(vv4.xy > -1.0) && all(vv4.xy < 1.0) && vv4.w > 0.0;

			if(bInsideFrustum && tryShadowmapFirst)
			{
				const F32 negativeZViewSpace = -mul(g_globalRendererConstants.m_matrices.m_view, Vec4(hitPos, 1.0)).z;
				const U32 shadowCascadeCount = dirLight.m_shadowCascadeCount;

				const U32 cascadeIdx = computeShadowCascadeIndex(negativeZViewSpace, dirLight.m_shadowCascadeDistances, shadowCascadeCount);

				shadow = computeShadowFactorDirLight<F32>(dirLight, cascadeIdx, hitPos, g_shadowAtlasTex, g_shadowSampler);
			}
			else
			{
				shadow = rayVisibility(hitPos, -dirLight.m_direction, shadowTMax, traceFlags) ? 0.0 : 1.0;
			}
		}

		// Do simple light shading

		const vector<T, 3> l = -dirLight.m_direction;
		const T lambert = max(T(0), dot(l, gbuffer.m_worldNormal));
		const vector<T, 3> diffC = diffuseLobe(gbuffer.m_diffuse);
		color += diffC * dirLight.m_diffuseColor * lambert * shadow;
	}

	// Local lights
	const LocalLightsGridConstants lightGrid = g_globalRendererConstants.m_localLightsGrid;
	if(all(hitPos < lightGrid.m_volumeMax) && all(hitPos > lightGrid.m_volumeMin))
	{
		const UVec3 cellId = (hitPos - lightGrid.m_volumeMin) / lightGrid.m_cellSize;
		const U32 cellIdx = cellId.z * lightGrid.m_cellCounts.x * lightGrid.m_cellCounts.y + cellId.y * lightGrid.m_cellCounts.x + cellId.x;

		// Compute an attenuation factor that will fade out the resulting color at the edges of the grid
		Vec3 a = (hitPos - lightGrid.m_volumeMin) / (lightGrid.m_volumeMax - lightGrid.m_volumeMin);
		a = abs(a * 2.0 - 1.0);
		a = 1.0 - a;
		const F32 gridEdgesAttenuation = sqrt(a.x * a.y * a.z);

		const U32 lightCount = SBUFF(g_lightIndexCountsPerCell, cellIdx);
		const U32 listOffset = SBUFF(g_lightIndexOffsetsPerCell, cellIdx);

		for(U32 i = 0; i < lightCount; ++i)
		{
			const U32 lightIdx = SBUFF(g_lightIndexList, listOffset + i);
			const GpuSceneLight light = SBUFF(g_lights, lightIdx);

			const Vec3 frag2Light = light.m_position - hitPos;
			const Vec3 nFrag2Light = normalize(frag2Light);

			F32 attenuation = computeAttenuationFactor(light.m_radius, frag2Light);
			if((U32)light.m_flags & (U32)GpuSceneLightFlag::kSpotLight)
			{
				attenuation *= computeSpotFactor(nFrag2Light, light.m_outerCos, light.m_innerCos, light.m_direction);
			}

			if(attenuation > kEpsilonF32 && doLocalLightShadow)
			{
				const F32 shadowFactor = rayVisibility(hitPos, nFrag2Light, length(frag2Light) - 0.1, traceFlags) ? 0.0 : 1.0;
				attenuation *= shadowFactor;
			}

			const T lambert = max(T(0), dot(nFrag2Light, gbuffer.m_worldNormal));
			const vector<T, 3> diffC = diffuseLobe(gbuffer.m_diffuse);
			color += diffC * light.m_diffuseColor * lambert * attenuation * gridEdgesAttenuation;
			// color += Vec3(0.5, 0, 0);
		}
	}

	return color;
}
#endif // ANKI_RAY_GEN_SHADER
