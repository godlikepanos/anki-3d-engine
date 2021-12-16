// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Calculates the motion vectors that will be used to sample from the previous frame

ANKI_SPECIALIZATION_CONSTANT_UVEC2(FB_SIZE, 0u);
const F32 MAX_REJECTION_DISTANCE = 2.0; // In meters
#define VARIANCE_CLIPPING 0
const F32 VARIANCE_CLIPPING_GAMMA = 2.0;

#include <AnKi/Shaders/Functions.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearClampSampler;
layout(set = 0, binding = 1) uniform texture2D u_currentDepthTex;
layout(set = 0, binding = 2) uniform texture2D u_historyDepthTex;
layout(set = 0, binding = 3) uniform texture2D u_velocityTex;

#if defined(ANKI_COMPUTE_SHADER)
layout(set = 0, binding = 4) uniform writeonly image2D u_motionVectorsImage;
layout(set = 0, binding = 5) uniform writeonly image2D u_rejectionFactorImage;

const UVec2 WORKGROUP_SIZE = UVec2(8, 8);
layout(local_size_x = WORKGROUP_SIZE.x, local_size_y = WORKGROUP_SIZE.y, local_size_z = 1) in;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec2 out_motionVectors;
layout(location = 1) out F32 out_rejectionFactor;
#endif

layout(push_constant, std140, row_major) uniform b_pc
{
	Mat4 u_reprojectionMat;
	Mat4 u_prevViewProjectionInvMat;
};

#define readDepth(texture_, uv, offsetX, offsetY) \
	textureLodOffset(sampler2D(texture_, u_linearClampSampler), uv, 0.0, IVec2(offsetX, offsetY)).r

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(WORKGROUP_SIZE, FB_SIZE))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / Vec2(FB_SIZE);
#else
	const Vec2 uv = in_uv;
#endif
	const F32 depth = readDepth(u_currentDepthTex, uv, 0, 0);

	//
	// Compute rejection factor
	//
	F32 rejection = 0.0;

	// Clamp history to neghbours of current pixel
	const F32 near0 = readDepth(u_currentDepthTex, uv, 1, 1);
	const F32 near1 = readDepth(u_currentDepthTex, uv, -1, 1);
	const F32 near2 = readDepth(u_currentDepthTex, uv, 1, -1);
	const F32 near3 = readDepth(u_currentDepthTex, uv, -1, -1);

#if VARIANCE_CLIPPING
	const F32 m1 = depth + near0 + near1 + near2 + near3;
	const F32 m2 = depth * depth + near0 * near0 + near1 * near1 + near2 * near2 + near3 * near3;

	const F32 mu = m1 / 5.0;
	const F32 sigma = sqrt(m2 / 5.0 - mu * mu);

	const F32 boxMin = mu - VARIANCE_CLIPPING_GAMMA * sigma;
	const F32 boxMax = mu + VARIANCE_CLIPPING_GAMMA * sigma;
#else
	const F32 boxMin = min(depth, min(near0, min(near1, min(near2, near3))));
	const F32 boxMax = max(depth, max(near0, max(near1, max(near2, near3))));
#endif

	const Vec2 velocity = textureLod(u_velocityTex, u_linearClampSampler, uv, 0.0).rg;

	Vec2 historyUv;
	if(velocity.x != 1.0)
	{
		historyUv = uv + velocity;
	}
	else
	{
		const Vec4 v4 = u_reprojectionMat * Vec4(UV_TO_NDC(uv), depth, 1.0);
		historyUv = NDC_TO_UV(v4.xy / v4.w);
	}

	const F32 historyDepth = readDepth(u_historyDepthTex, historyUv, 0, 0);
	const F32 clampedHistoryDepth = clamp(historyDepth, boxMin, boxMax);

	// This factor shows when new pixels appeared by checking depth differences
	const Vec4 historyViewSpace4 = u_prevViewProjectionInvMat * Vec4(UV_TO_NDC(historyUv), historyDepth, 1.0);
	const Vec3 historyViewSpace = historyViewSpace4.xyz / historyViewSpace4.w;

	const Vec4 clampedHistoryViewSpace4 =
		u_prevViewProjectionInvMat * Vec4(UV_TO_NDC(historyUv), clampedHistoryDepth, 1.0);
	const Vec3 clampedHistoryViewSpace = clampedHistoryViewSpace4.xyz / clampedHistoryViewSpace4.w;

	const Vec3 delta = clampedHistoryViewSpace - historyViewSpace;
	const F32 distSquared = dot(delta, delta);
	const F32 disocclusionFactor = min(1.0, distSquared / (MAX_REJECTION_DISTANCE * MAX_REJECTION_DISTANCE));
	rejection = max(rejection, disocclusionFactor);

	// New pixels might appeared, add them to the disocclusion
	const F32 minUv = min(historyUv.x, historyUv.y);
	const F32 maxUv = max(historyUv.x, historyUv.y);
	if(minUv <= 0.0 || maxUv >= 1.0)
	{
		rejection = 1.0;
	}

	// Write out
#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_motionVectorsImage, IVec2(gl_GlobalInvocationID.xy), Vec4(historyUv - uv, 0.0, 0.0));
	imageStore(u_rejectionFactorImage, IVec2(gl_GlobalInvocationID.xy), Vec4(rejection, 0.0, 0.0, 0.0));
#else
	out_motionVectors = historyUv - uv;
	out_rejectionFactor = rejection;
#endif
}
