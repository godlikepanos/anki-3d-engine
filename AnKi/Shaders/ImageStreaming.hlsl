// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/ImageStreaming.h>
#include <AnKi/Shaders/ImportanceSampling.hlsl>

// Standard LOD calculation as described in the GL spec
F32 computeLodAnisoGL(Vec2 uv, Vec2 texSize, F32 lodBias, F32 maxAniso)
{
	const Vec2 dUdx = ddx(uv);
	const Vec2 dUdy = ddy(uv);

	// Derivatives in texel space
	const Vec2 dx = dUdx * texSize; // (dU/dx * width, dV/dx * height)
	const Vec2 dy = dUdy * texSize;

	const F32 px = length(dx);
	const F32 py = length(dy);

	const F32 pmax = max(px, py);
	const F32 pmin = min(px, py);

	const F32 ratio = pmax / max(pmin, kEpsilonF32);

	// The HW takes an integer number of samples along the major axis so round up
	const F32 N = clamp(ceil(ratio), 1.0, maxAniso);

	const F32 lambda = log2(max(pmax / N, kEpsilonF32));

	return lambda + lodBias;
}

// Taken from nVidia's stochastic texture filtering SDK
F32 computeTextureLodSTF(Vec2 dim, Vec4 textureGrads, F32 minLod, F32 maxLod, F32 mipBias, F32 maxAnisotropy)
{
	const F32 dudx = dim.x * textureGrads.x;
	const F32 dvdx = dim.y * textureGrads.y;
	const F32 dudy = dim.x * textureGrads.z;
	const F32 dvdy = dim.y * textureGrads.w;

	Vec2 maxAxis = Vec2(dudy, dvdy);
	Vec2 minAxis = Vec2(dudx, dvdx);

	if(dot(minAxis, minAxis) > dot(maxAxis, maxAxis))
	{
		minAxis = Vec2(dudy, dvdy);
		maxAxis = Vec2(dudx, dvdx);
	}

	const F32 maxAxisLength = length(maxAxis);
	F32 minAxisLength = length(minAxis);

	// Clamp the anisotropy ratio by growing the minor axis
	if(minAxisLength > 0.0 && (minAxisLength * maxAnisotropy) < maxAxisLength)
	{
		const F32 scale = maxAxisLength / (minAxisLength * maxAnisotropy);
		minAxisLength *= scale;
	}

	const F32 log2MinAxis = (minAxisLength > kEpsilonF32) ? log2(minAxisLength) : minLod;
	const F32 mipValue = clamp(log2MinAxis + mipBias, minLod, maxLod);
	return mipValue;
}

// A wrapper on top of computeTextureLodSTF
F32 computeLodAnisoSTF(Vec2 uv, Vec2 texSize, F32 lodBias, F32 maxAniso)
{
	return computeTextureLodSTF(texSize, Vec4(ddx(uv), ddy(uv)), 0.0, 100.0, lodBias, maxAniso);
}

// Isotropic LOD calculation taken from the GL spec
F32 computeLodIsotropic(Vec2 uv, Vec2 texDim, F32 lodBias)
{
	const Vec2 dUVdx = ddx(uv);
	const Vec2 dUVdy = ddy(uv);

	// Derivatives in texel space
	const Vec2 dX = dUVdx * texDim;
	const Vec2 dY = dUVdy * texDim;

	// The footprint scale. The max() also avoids log2(0)
	const F32 rho = max(max(length(dX), length(dY)), kEpsilonF32);

	return log2(rho) + lodBias; // May be negative for magnification
}

// Stochastic texture sampling
template<typename TRandGenerator>
Vec4 sampleTexture2D(ImageDescriptor desc, SamplerState sampl, Vec2 uv, F32 lodBias, F32 anisotropy, inout TRandGenerator randg)
{
	F32 lod = computeLodAnisoGL(uv, Vec2(desc.m_width, desc.m_height), lodBias, anisotropy);

	lod = (frac(lod) < rand(randg)) ? floor(lod) : ceil(lod);
	lod = max(0.0, lod);

	U32 arrIdx;
	const U32 lodu = lod;
	F32 texLod;
	if(lodu < desc.m_firstMipmapOfTailChain)
	{
		arrIdx = max(desc.m_firstMipmap, lodu);
		texLod = 0.0;
	}
	else
	{
		arrIdx = desc.m_firstMipmapOfTailChain;
		texLod = lodu - desc.m_firstMipmapOfTailChain;
	}

	const U32 bindlessIndex = desc.m_mipmapTextureIndices[arrIdx];
	const Vec4 final = getBindlessTextureNonUniformIndex2DVec4(bindlessIndex).SampleLevel(sampl, uv, texLod);

	return final;
}
