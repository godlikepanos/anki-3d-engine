// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Calculates the motion vectors that will be used to sample from the previous frame

#pragma anki hlsl

#include <AnKi/Shaders/Functions.hlsl>

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 0u);
constexpr F32 kMaxRejectionDistance = 0.1; // In meters
constexpr F32 kMaxHistoryLength = 16.0;

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D g_currentDepthTex;
[[vk::binding(2)]] Texture2D g_historyDepthTex;
[[vk::binding(3)]] Texture2D g_velocityTex;
[[vk::binding(4)]] Texture2D g_historyLengthTex;

struct Uniforms
{
	Mat4 m_reprojectionMat;
	Mat4 m_viewProjectionInvMat;
	Mat4 m_prevViewProjectionInvMat;
};

[[vk::binding(5)]] ConstantBuffer<Uniforms> g_unis;

#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(6)]] RWTexture2D<Vec2> g_motionVectorsUav;
[[vk::binding(7)]] RWTexture2D<F32> g_historyLengthUav;
#endif

Vec3 clipToWorld(Vec4 clip, Mat4 clipToWorldMat)
{
	const Vec4 v4 = mul(clipToWorldMat, clip);
	return v4.xyz / v4.w;
}

/// Average the some depth values and unproject.
Vec3 getAverageWorldPosition(Texture2D tex, Vec2 uv, Mat4 clipToWorldMat)
{
	const Vec2 halfTexel = (1.0 / Vec2(kFramebufferSize)) / 2.0;

	Vec4 depths = tex.GatherRed(g_linearAnyClampSampler, uv + halfTexel);
	depths += tex.GatherRed(g_linearAnyClampSampler, uv - halfTexel);

	const F32 avgDepth = (depths.x + depths.y + depths.z + depths.w) / 8.0;

	return clipToWorld(Vec4(uvToNdc(uv), avgDepth, 1.0), clipToWorldMat);
}

/// Get the depths of some neighbour texels, unproject and find the AABB in world space that encloses them.
void getMinMaxWorldPositions(Texture2D tex, Vec2 uv, Mat4 clipToWorldMat, out Vec3 aabbMin, out Vec3 aabbMax)
{
	const Vec2 halfTexel = (1.0 / Vec2(kFramebufferSize)) / 2.0;

	const Vec4 depths1 = tex.GatherRed(g_linearAnyClampSampler, uv + halfTexel);
	const Vec4 depths2 = tex.GatherRed(g_linearAnyClampSampler, uv - halfTexel);

	const Vec4 minDepths4 = min(depths1, depths2);
	const Vec4 maxDepths4 = max(depths1, depths2);

	const Vec2 minDepths2 = min(minDepths4.xy, minDepths4.zw);
	const Vec2 maxDepths2 = max(maxDepths4.xy, maxDepths4.zw);

	const F32 minDepth = min(minDepths2.x, minDepths2.y);
	const F32 maxDepth = max(maxDepths2.x, maxDepths2.y);

	const Vec3 a = clipToWorld(Vec4(uvToNdc(uv), minDepth, 1.0), clipToWorldMat);
	const Vec3 b = clipToWorld(Vec4(uvToNdc(uv), maxDepth, 1.0), clipToWorldMat);

	aabbMin = min(a, b);
	aabbMax = max(a, b);
}

F32 computeRejectionFactor(Vec2 uv, Vec2 historyUv)
{
	Vec3 boxMin;
	Vec3 boxMax;
	getMinMaxWorldPositions(g_currentDepthTex, uv, g_unis.m_viewProjectionInvMat, boxMin, boxMax);

#if 0
	const F32 historyDepth = g_historyDepthTex.SampleLevel(g_linearAnyClampSampler, historyUv, 0.0).r;
	const Vec3 historyWorldPos = clipToWorld(Vec4(uvToNdc(historyUv), historyDepth, 1.0), g_unis.m_prevViewProjectionInvMat);
#else
	// Average gives more rejection so less ghosting
	const Vec3 historyWorldPos =
		getAverageWorldPosition(g_historyDepthTex, historyUv, g_unis.m_prevViewProjectionInvMat);
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

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
struct FragOut
{
	Vec2 m_motionVectors : SV_TARGET0;
	F32 m_historyLength : SV_TARGET1;
};

FragOut main(Vec2 uv : TEXCOORD)
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(UVec2(8, 8), kFramebufferSize, svDispatchThreadId.xy))
	{
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(kFramebufferSize);
#endif
	const F32 depth = g_currentDepthTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).r;

	const Vec2 velocity = g_velocityTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).rg;

	Vec2 historyUv;
	if(velocity.x != 1.0)
	{
		historyUv = uv + velocity;
	}
	else
	{
		const Vec4 v4 = mul(g_unis.m_reprojectionMat, Vec4(uvToNdc(uv), depth, 1.0));
		historyUv = ndcToUv(v4.xy / v4.w);
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
		historyLength = g_historyLengthTex.SampleLevel(g_linearAnyClampSampler, historyUv, 0.0).r;
		historyLength += 1.0 / kMaxHistoryLength;
	}

	// Write out
#if defined(ANKI_COMPUTE_SHADER)
	g_motionVectorsUav[svDispatchThreadId.xy] = historyUv - uv;
	g_historyLengthUav[svDispatchThreadId.xy] = historyLength;
#else
	FragOut output;
	output.m_motionVectors = historyUv - uv;
	output.m_historyLength = historyLength;
	return output;
#endif
}
