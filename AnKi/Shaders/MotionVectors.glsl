// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Calculates the motion vectors that will be used to sample from the previous frame

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 0u);
const F32 kMaxRejectionDistance = 0.1; // In meters
const F32 kMaxHistoryLength = 16.0;

#include <AnKi/Shaders/Functions.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform texture2D u_currentDepthTex;
layout(set = 0, binding = 2) uniform texture2D u_historyDepthTex;
layout(set = 0, binding = 3) uniform texture2D u_velocityTex;
layout(set = 0, binding = 4) uniform texture2D u_historyLengthTex;

struct Uniforms
{
	Mat4 m_reprojectionMat;
	Mat4 m_viewProjectionInvMat;
	Mat4 m_prevViewProjectionInvMat;
};

layout(set = 0, binding = 5, std140, row_major) uniform b_unis
{
	Uniforms u_unis;
};

#if defined(ANKI_COMPUTE_SHADER)
layout(set = 0, binding = 6) uniform writeonly image2D u_motionVectorsImage;
layout(set = 0, binding = 7) uniform writeonly image2D u_historyLengthImage;

const UVec2 kWorkgroupSize = UVec2(8, 8);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y, local_size_z = 1) in;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec2 out_motionVectors;
layout(location = 1) out F32 out_historyLength;
#endif

Vec3 clipToWorld(Vec4 clip, Mat4 clipToWorldMat)
{
	const Vec4 v4 = clipToWorldMat * clip;
	return v4.xyz / v4.w;
}

/// Average the some depth values and unproject.
Vec3 getAverageWorldPosition(texture2D tex, Vec2 uv, Mat4 clipToWorldMat)
{
	const Vec2 halfTexel = (1.0 / Vec2(kFramebufferSize)) / 2.0;

	Vec4 depths = textureGather(sampler2D(tex, u_linearAnyClampSampler), uv + halfTexel, 0);
	depths += textureGather(sampler2D(tex, u_linearAnyClampSampler), uv - halfTexel, 0);

	const F32 avgDepth = (depths.x + depths.y + depths.z + depths.w) / 8.0;

	return clipToWorld(Vec4(UV_TO_NDC(uv), avgDepth, 1.0), clipToWorldMat);
}

/// Get the depths of some neighbour texels, unproject and find the AABB in world space that encloses them.
void getMinMaxWorldPositions(texture2D tex, Vec2 uv, Mat4 clipToWorldMat, out Vec3 aabbMin, out Vec3 aabbMax)
{
	const Vec2 halfTexel = (1.0 / Vec2(kFramebufferSize)) / 2.0;

	const Vec4 depths1 = textureGather(sampler2D(tex, u_linearAnyClampSampler), uv + halfTexel, 0);
	const Vec4 depths2 = textureGather(sampler2D(tex, u_linearAnyClampSampler), uv - halfTexel, 0);

	const Vec4 minDepths4 = min(depths1, depths2);
	const Vec4 maxDepths4 = max(depths1, depths2);

	const Vec2 minDepths2 = min(minDepths4.xy, minDepths4.zw);
	const Vec2 maxDepths2 = max(maxDepths4.xy, maxDepths4.zw);

	const F32 minDepth = min(minDepths2.x, minDepths2.y);
	const F32 maxDepth = max(maxDepths2.x, maxDepths2.y);

	const Vec3 a = clipToWorld(Vec4(UV_TO_NDC(uv), minDepth, 1.0), clipToWorldMat);
	const Vec3 b = clipToWorld(Vec4(UV_TO_NDC(uv), maxDepth, 1.0), clipToWorldMat);

	aabbMin = min(a, b);
	aabbMax = max(a, b);
}

F32 computeRejectionFactor(Vec2 uv, Vec2 historyUv)
{
	Vec3 boxMin;
	Vec3 boxMax;
	getMinMaxWorldPositions(u_currentDepthTex, uv, u_unis.m_viewProjectionInvMat, boxMin, boxMax);

#if 0
	const F32 historyDepth = textureLod(u_historyDepthTex, u_linearAnyClampSampler, historyUv, 0.0).r;
	const Vec3 historyWorldPos = clipToWorld(Vec4(UV_TO_NDC(historyUv), historyDepth, 1.0), u_unis.m_prevViewProjectionInvMat);
#else
	// Average gives more rejection so less ghosting
	const Vec3 historyWorldPos =
		getAverageWorldPosition(u_historyDepthTex, historyUv, u_unis.m_prevViewProjectionInvMat);
#endif
	const Vec3 clampedHistoryWorldPos = clamp(historyWorldPos, boxMin, boxMax);

	// This factor shows when new pixels appeared by checking depth differences
	const Vec3 delta = clampedHistoryWorldPos - historyWorldPos;
	const F32 distSquared = dot(delta, delta);
	const F32 disocclusionFactor = min(1.0, distSquared / (kMaxRejectionDistance * kMaxRejectionDistance));
	F32 rejection = disocclusionFactor;

	// New pixels might appeared, add them to the disocclusion
	const F32 minUv = min(historyUv.x, historyUv.y);
	const F32 maxUv = max(historyUv.x, historyUv.y);
	if(minUv <= 0.0 || maxUv >= 1.0)
	{
		rejection = 1.0;
	}

	return rejection;
}

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, kFramebufferSize))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(kFramebufferSize);
#else
	const Vec2 uv = in_uv;
#endif
	const F32 depth = textureLod(u_currentDepthTex, u_linearAnyClampSampler, uv, 0.0).r;

	const Vec2 velocity = textureLod(u_velocityTex, u_linearAnyClampSampler, uv, 0.0).rg;

	Vec2 historyUv;
	if(velocity.x != 1.0)
	{
		historyUv = uv + velocity;
	}
	else
	{
		const Vec4 v4 = u_unis.m_reprojectionMat * Vec4(UV_TO_NDC(uv), depth, 1.0);
		historyUv = NDC_TO_UV(v4.xy / v4.w);
	}

	const F32 rejection = computeRejectionFactor(uv, historyUv);

	// Compute history length
	F32 historyLength;
	if(rejection >= 0.5)
	{
		// Rejection factor too high, reset the temporal history
		historyLength = 1.0 / kMaxHistoryLength;
	}
	else
	{
		historyLength = textureLod(u_historyLengthTex, u_linearAnyClampSampler, historyUv, 0.0).r;
		historyLength += 1.0 / kMaxHistoryLength;
	}

	// Write out
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_motionVectorsImage, IVec2(gl_GlobalInvocationID.xy), Vec4(historyUv - uv, 0.0, 0.0));
	imageStore(u_historyLengthImage, IVec2(gl_GlobalInvocationID.xy), Vec4(historyLength, 0.0, 0.0, 0.0));
#else
	out_motionVectors = historyUv - uv;
	out_historyLength = historyLength;
#endif
}
