// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/Common.hlsl>

#pragma anki mutator ORIENTATION 0 1 2 // 0: VERTICAL, 1: HORIZONTAL, 2: BOX
#pragma anki mutator SAMPLE_COUNT 3 5 7 9 11 13 15
#pragma anki mutator COLOR_COMPONENTS 4 3 1

#define ORIENTATION_VERTICAL 0
#define ORIENTATION_HORIZONTAL 1
#define ORIENTATION_BOX 2

#if SAMPLE_COUNT < 3
#	error See file
#endif

// Define some macros depending on the number of components
#if COLOR_COMPONENTS == 4
typedef Vec4 ColorType;
#elif COLOR_COMPONENTS == 3
typedef Vec3 ColorType;
#elif COLOR_COMPONENTS == 1
typedef F32 ColorType;
#else
#	error See file
#endif

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<ColorType> g_inTex;
[[vk::binding(2)]] Texture2D g_depthTex;

#if defined(ANKI_COMPUTE_SHADER)
#	define THREADGROUP_SQRT_SIZE 8
[[vk::binding(2)]] RWTexture2D<ColorType> g_outImg;
#endif

F32 computeDepthWeight(F32 refDepth, F32 depth)
{
	const F32 diff = abs(refDepth - depth);
	const F32 weight = 1.0 / (kEpsilonF32 + diff);
	return sqrt(weight);
}

F32 readDepth(Vec2 uv)
{
	return g_depthTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0).r;
}

void sampleTex(Vec2 uv, F32 refDepth, inout ColorType col, inout F32 weight)
{
	const ColorType color = g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0);
	const F32 w = computeDepthWeight(refDepth, readDepth(uv));
	col += color * w;
	weight += w;
}

#if defined(ANKI_COMPUTE_SHADER)
ANKI_NUMTHREADS(THREADGROUP_SQRT_SIZE, THREADGROUP_SQRT_SIZE, 1)
void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
ColorType main([[vk::location(0)]] Vec2 uv : TEXCOORD): SV_TARGET0
#endif
{
	UVec2 textureSize;
	U32 mipCount;
	g_inTex.GetDimensions(0, textureSize.x, textureSize.y, mipCount);

	// Set UVs
#if defined(ANKI_COMPUTE_SHADER)
	[[branch]] if(svDispatchThreadId.x >= textureSize.x || svDispatchThreadId.y >= textureSize.y)
	{
		// Out of bounds
		return;
	}

	const Vec2 uv = (Vec2(svDispatchThreadId.xy) + 0.5) / Vec2(textureSize);
#endif

	const Vec2 texelSize = 1.0 / Vec2(textureSize);

	// Sample
	ColorType color = g_inTex.SampleLevel(g_linearAnyClampSampler, uv, 0.0);
	const F32 refDepth = readDepth(uv);
	F32 weight = 1.0;

#if ORIENTATION != ORIENTATION_BOX
	// Do seperable

#	if ORIENTATION == ORIENTATION_HORIZONTAL
#		define X_OR_Y x
#	else
#		define X_OR_Y y
#	endif

	Vec2 uvOffset = Vec2(0.0, 0.0);
	uvOffset.X_OR_Y = 1.5 * texelSize.X_OR_Y;

	[[unroll]] for(U32 i = 0u; i < (SAMPLE_COUNT - 1u) / 2u; ++i)
	{
		sampleTex(uv + uvOffset, refDepth, color, weight);
		sampleTex(uv - uvOffset, refDepth, color, weight);

		uvOffset.X_OR_Y += 2.0 * texelSize.X_OR_Y;
	}
#else
	// Do box

	const Vec2 offset = 1.5 * texelSize;

	sampleTex(uv + Vec2(+offset.x, +offset.y), refDepth, color, weight);
	sampleTex(uv + Vec2(+offset.x, -offset.y), refDepth, color, weight);
	sampleTex(uv + Vec2(-offset.x, +offset.y), refDepth, color, weight);
	sampleTex(uv + Vec2(-offset.x, -offset.y), refDepth, color, weight);

	sampleTex(uv + Vec2(offset.x, 0.0), refDepth, color, weight);
	sampleTex(uv + Vec2(0.0, offset.y), refDepth, color, weight);
	sampleTex(uv + Vec2(-offset.x, 0.0), refDepth, color, weight);
	sampleTex(uv + Vec2(0.0, -offset.y), refDepth, color, weight);
#endif

	color /= weight;

	// Write value
#if defined(ANKI_COMPUTE_SHADER)
	g_outImg[svDispatchThreadId.xy] = color;
#else
	return color;
#endif
}
