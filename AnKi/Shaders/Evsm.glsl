// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_SPECIALIZATION_CONSTANT_UVEC2(kInputTextureSize, 0u);
ANKI_SPECIALIZATION_CONSTANT_UVEC2(kFramebufferSize, 2u);

#include <AnKi/Shaders/Include/MiscRendererTypes.h>

layout(set = 0, binding = 0) readonly buffer b_unis
{
	EvsmResolveUniforms u_uniforms[];
};

#if defined(ANKI_VERTEX_SHADER)
#	include <AnKi/Shaders/Common.glsl>

layout(location = 0) out Vec2 out_uv;
layout(location = 1) flat out I32 out_instanceIndex;

void main()
{
	const EvsmResolveUniforms uni = u_uniforms[gl_InstanceIndex];

	const Vec2 uv = Vec2(((gl_VertexID + 2) / 3) % 2, ((gl_VertexID + 1) / 3) % 2);

	out_uv = uv * uni.m_uvScale + uni.m_uvTranslation;

	const Vec2 pos = UV_TO_NDC((Vec2(uni.m_viewportXY) + uni.m_viewportZW * uv) / Vec2(kFramebufferSize));
	gl_Position = Vec4(pos, 0.0, 1.0);

	out_instanceIndex = gl_InstanceIndex;
}

#else // !defined(ANKI_VERTEX_SHADER)

#	include <AnKi/Shaders/GaussianBlurCommon.glsl>
#	include <AnKi/Shaders/LightFunctions.glsl>

const F32 kOffset = 1.25;

layout(set = 0, binding = 1) uniform sampler u_linearAnyClampSampler;
layout(set = 0, binding = 2) uniform texture2D u_inputTex;

#	if defined(ANKI_COMPUTE_SHADER)
layout(set = 0, binding = 3) uniform writeonly image2D u_outImg;

layout(local_size_x = 8, local_size_y = 8) in;
#	else
layout(location = 0) in Vec2 in_uv;
layout(location = 1) flat in I32 in_instanceIndex;
layout(location = 0) out Vec4 out_moments;
#	endif

Vec4 computeMoments(Vec2 uv)
{
	const F32 d = textureLod(u_inputTex, u_linearAnyClampSampler, uv, 0.0).r;
	const Vec2 posAndNeg = evsmProcessDepth(d);
	return Vec4(posAndNeg.x, posAndNeg.x * posAndNeg.x, posAndNeg.y, posAndNeg.y * posAndNeg.y);
}

void main()
{
#	if defined(ANKI_COMPUTE_SHADER)
	const EvsmResolveUniforms uni = u_uniforms[gl_GlobalInvocationID.z];

	Vec2 uv = (Vec2(gl_GlobalInvocationID.xy) + 0.5) / uni.m_viewportZW; // in [0, 1]
	uv = uv * uni.m_uvScale + uni.m_uvTranslation;
#	else
	const EvsmResolveUniforms uni = u_uniforms[in_instanceIndex];

	Vec2 uv = in_uv;
#	endif

	// Compute the UV limits. We can't sample beyond those
	const Vec2 kTexelSize = 1.0 / Vec2(kInputTextureSize);
	const Vec2 kHalfTexelSize = kTexelSize / 2.0;
	const Vec2 maxUv = uni.m_uvMax - kHalfTexelSize;
	const Vec2 minUv = uni.m_uvMin + kHalfTexelSize;

	// Sample
	const Vec2 kUvOffset = kOffset * kTexelSize;
	const F32 w0 = kBoxWeights[0u];
	const F32 w1 = kBoxWeights[1u];
	const F32 w2 = kBoxWeights[2u];
	Vec4 moments;
	if(uni.m_blur != 0u)
	{
		moments = computeMoments(uv) * w0;
		moments += computeMoments(clamp(uv + Vec2(kUvOffset.x, 0.0), minUv, maxUv)) * w1;
		moments += computeMoments(clamp(uv + Vec2(-kUvOffset.x, 0.0), minUv, maxUv)) * w1;
		moments += computeMoments(clamp(uv + Vec2(0.0, kUvOffset.y), minUv, maxUv)) * w1;
		moments += computeMoments(clamp(uv + Vec2(0.0, -kUvOffset.y), minUv, maxUv)) * w1;
		moments += computeMoments(clamp(uv + Vec2(kUvOffset.x, kUvOffset.y), minUv, maxUv)) * w2;
		moments += computeMoments(clamp(uv + Vec2(-kUvOffset.x, kUvOffset.y), minUv, maxUv)) * w2;
		moments += computeMoments(clamp(uv + Vec2(kUvOffset.x, -kUvOffset.y), minUv, maxUv)) * w2;
		moments += computeMoments(clamp(uv + Vec2(-kUvOffset.x, -kUvOffset.y), minUv, maxUv)) * w2;
	}
	else
	{
		moments = computeMoments(uv);
	}

	// Write the results
#	if ANKI_EVSM4
	const Vec4 outColor = moments;
#	else
	const Vec4 outColor = Vec4(moments.xy, 0.0, 0.0);
#	endif

#	if defined(ANKI_COMPUTE_SHADER)
	imageStore(u_outImg, IVec2(gl_GlobalInvocationID.xy) + uni.m_viewportXY, outColor);
#	else
	out_moments = outColor;
#	endif
}

#endif // !defined(ANKI_VERTEX_SHADER)
