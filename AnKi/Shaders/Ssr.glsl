// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator STOCHASTIC 0 1
#pragma anki mutator EXTRA_REJECTION 0 1

#include <AnKi/Shaders/LightFunctions.glsl>
#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/Include/SsrTypes.h>
#include <AnKi/Shaders/TonemappingFunctions.glsl>
#include <AnKi/Shaders/SsRaymarching.glsl>

layout(set = 0, binding = 0, row_major) uniform b_unis
{
	SsrUniforms u_unis;
};

layout(set = 0, binding = 1) uniform sampler u_trilinearClampSampler;
layout(set = 0, binding = 2) uniform texture2D u_gbufferRt1;
layout(set = 0, binding = 3) uniform texture2D u_gbufferRt2;
layout(set = 0, binding = 4) uniform texture2D u_depthRt;
layout(set = 0, binding = 5) uniform texture2D u_lightBufferRt;

layout(set = 0, binding = 6) uniform sampler u_trilinearRepeatSampler;
layout(set = 0, binding = 7) uniform texture2D u_noiseTex;
const Vec2 NOISE_TEX_SIZE = Vec2(64.0);

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 WORKGROUP_SIZE = UVec2(8, 8);
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;

layout(set = 0, binding = 8) uniform writeonly image2D out_img;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec4 out_color;
#endif

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(WORKGROUP_SIZE, u_unis.m_framebufferSize))
	{
		return;
	}
	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(u_unis.m_framebufferSize);
#else
	const Vec2 uv = in_uv;
#endif

	// Read part of the G-buffer
	const F32 roughness = unpackRoughnessFromGBuffer(textureLod(u_gbufferRt1, u_trilinearClampSampler, uv, 0.0));
	const Vec3 worldNormal = unpackNormalFromGBuffer(textureLod(u_gbufferRt2, u_trilinearClampSampler, uv, 0.0));

	// Get depth
	const F32 depth = textureLod(u_depthRt, u_trilinearClampSampler, uv, 0.0).r;

	// Rand idx
	const Vec2 noiseUv = Vec2(u_unis.m_framebufferSize) / NOISE_TEX_SIZE * uv;
	const Vec3 noise =
		animateBlueNoise(textureLod(u_noiseTex, u_trilinearRepeatSampler, noiseUv, 0.0).rgb, u_unis.m_frameCount % 8u);

	// Get view pos
	const Vec4 viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(uv), depth, 1.0);
	const Vec3 viewPos = viewPos4.xyz / viewPos4.w;

	// Compute refl vector
	const Vec3 viewDir = normalize(viewPos);
	const Vec3 viewNormal = u_unis.m_normalMat * worldNormal;
#if STOCHASTIC
	const Vec3 reflVec = sampleReflectionVector(viewDir, viewNormal, roughness, noise.xy);
#else
	const Vec3 reflVec = reflect(viewDir, viewNormal);
#endif

	// Do the heavy work
	Vec3 hitPoint;
	F32 hitAttenuation;
	const U32 lod = 0u;
	const U32 step = u_unis.m_firstStepPixels;
	const F32 stepf = F32(step);
	const F32 minStepf = stepf / 4.0;
	raymarchGroundTruth(viewPos, reflVec, uv, depth, u_unis.m_projMat, u_unis.m_maxSteps, u_depthRt,
						u_trilinearClampSampler, F32(lod), u_unis.m_depthBufferSize, step,
						U32((stepf - minStepf) * noise.x + minStepf), hitPoint, hitAttenuation);

#if EXTRA_REJECTION
	// Reject backfacing
	ANKI_BRANCH if(hitAttenuation > 0.0)
	{
		const Vec3 hitNormal =
			u_unis.m_normalMat
			* unpackNormalFromGBuffer(textureLod(u_gbufferRt2, u_trilinearClampSampler, hitPoint.xy, 0.0));
		F32 backFaceAttenuation;
		rejectBackFaces(reflVec, hitNormal, backFaceAttenuation);

		hitAttenuation *= backFaceAttenuation;
	}

	// Reject far from hit point
	ANKI_BRANCH if(hitAttenuation > 0.0)
	{
		const F32 depth = textureLod(u_depthRt, u_trilinearClampSampler, hitPoint.xy, 0.0).r;
		Vec4 viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(hitPoint.xy), depth, 1.0);
		const F32 actualZ = viewPos4.z / viewPos4.w;

		viewPos4 = u_unis.m_invProjMat * Vec4(UV_TO_NDC(hitPoint.xy), hitPoint.z, 1.0);
		const F32 hitZ = viewPos4.z / viewPos4.w;

		const F32 rejectionMeters = 1.0;
		const F32 diff = abs(actualZ - hitZ);
		const F32 distAttenuation = (diff < rejectionMeters) ? 1.0 : 0.0;
		hitAttenuation *= distAttenuation;
	}
#endif

	// Read the reflection
	Vec4 outColor;
	ANKI_BRANCH if(hitAttenuation > 0.0)
	{
		// Reproject the UV because you are reading the previous frame
		const Vec4 v4 = u_unis.m_prevViewProjMatMulInvViewProjMat * Vec4(UV_TO_NDC(hitPoint.xy), hitPoint.z, 1.0);
		hitPoint.xy = NDC_TO_UV(v4.xy / v4.w);

#if STOCHASTIC
		// LOD stays 0
		const F32 lod = 0.0;
#else
		// Compute the LOD based on the roughness
		const F32 lod = F32(u_unis.m_lightBufferMipCount - 1u) * roughness;
#endif

		// Read the light buffer
		outColor.rgb = textureLod(u_lightBufferRt, u_trilinearClampSampler, hitPoint.xy, lod).rgb;
		outColor.rgb = clamp(outColor.rgb, 0.0, MAX_F32); // Fix the value just in case
		outColor.rgb *= hitAttenuation;
		outColor.a = 1.0 - hitAttenuation;
	}
	else
	{
		outColor = Vec4(0.0, 0.0, 0.0, 1.0);
	}

	// Store
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(out_img, IVec2(gl_GlobalInvocationID.xy), outColor);
#else
	out_color = outColor;
#endif
}
