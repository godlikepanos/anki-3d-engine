// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki mutator BLUR_ORIENTATION 0 1 // 0: in X axis, 1: in Y axis

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/PackFunctions.glsl>
#include <AnKi/Shaders/Functions.glsl>
#include <AnKi/Shaders/BilateralFilter.glsl>

layout(set = 0, binding = 0) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 1) uniform ANKI_RP texture2D u_toDenoiseTex;
layout(set = 0, binding = 2) uniform texture2D u_depthTex;

#if defined(ANKI_COMPUTE_SHADER)
const UVec2 kWorkgroupSize = UVec2(8u, 8u);
layout(local_size_x = kWorkgroupSize.x, local_size_y = kWorkgroupSize.y) in;

layout(set = 0, binding = 3) writeonly uniform ANKI_RP image2D u_outImg;
#else
layout(location = 0) in Vec2 in_uv;
layout(location = 0) out Vec3 out_color;
#endif

layout(push_constant, std430, row_major) uniform b_pc
{
	IndirectDiffuseDenoiseUniforms u_unis;
};

Vec3 unproject(Vec2 ndc, F32 depth)
{
	const Vec4 worldPos4 = u_unis.m_invertedViewProjectionJitterMat * Vec4(ndc, depth, 1.0);
	const Vec3 worldPos = worldPos4.xyz / worldPos4.w;
	return worldPos;
}

void main()
{
#if defined(ANKI_COMPUTE_SHADER)
	if(skipOutOfBoundsInvocations(kWorkgroupSize, u_unis.m_viewportSize))
	{
		return;
	}

	const Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / u_unis.m_viewportSizef;
#else
	const Vec2 uv = in_uv;
#endif

	// Reference
	const F32 depthCenter = textureLod(u_depthTex, u_linearAnyClampSampler, uv, 0.0).r;
	if(depthCenter == 1.0)
	{
#if defined(ANKI_COMPUTE_SHADER)
		imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), Vec4(0.0));
#else
		out_color = Vec3(0.0);
#endif
		return;
	}

	const Vec3 positionCenter = unproject(UV_TO_NDC(uv), depthCenter);

	// Sample
	ANKI_RP F32 weight = kEpsilonRp;
	ANKI_RP Vec3 color = Vec3(0.0);

	for(F32 i = -u_unis.m_sampleCountDiv2; i <= u_unis.m_sampleCountDiv2; i += 1.0)
	{
		const Vec2 texelSize = 1.0 / u_unis.m_viewportSizef;
#if BLUR_ORIENTATION == 0
		const Vec2 sampleUv = Vec2(uv.x + i * texelSize.x, uv.y);
#else
		const Vec2 sampleUv = Vec2(uv.x, uv.y + i * texelSize.y);
#endif

		const F32 depthTap = textureLod(u_depthTex, u_linearAnyClampSampler, sampleUv, 0.0).r;

		ANKI_RP F32 w = calculateBilateralWeightDepth(depthCenter, depthTap, 1.0);
		// w *= gaussianWeight(0.4, abs(F32(i)) / (u_unis.m_sampleCountDiv2 * 2.0 + 1.0));
		weight += w;

		color += textureLod(u_toDenoiseTex, u_linearAnyClampSampler, sampleUv, 0.0).xyz * w;
	}

	// Normalize and store
	color /= weight;

#if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), Vec4(color, 0.0));
	// imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy), textureLod(u_toDenoiseTex, u_linearAnyClampSampler, uv,
	// 0.0));
#else
	out_color = color;
#endif
}
