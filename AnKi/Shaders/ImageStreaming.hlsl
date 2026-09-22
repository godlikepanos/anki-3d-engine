// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/ImageStreaming.h>

static I32 g_detailedImageLodFeedback = 0;

// Standard LOD calculation as described in the GL spec
F32 computeTextureLodAnisoGL(Vec2 texSize, Vec2 dUdx, Vec2 dUdy, F32 lodBias, F32 maxAniso)
{
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
F32 computeTextureLodAnisoSTF(Vec2 texSize, Vec2 dUdx, Vec2 dUdy, F32 minLod, F32 maxLod, F32 mipBias, F32 maxAnisotropy)
{
	const F32 dudx = texSize.x * dUdx.x;
	const F32 dvdx = texSize.y * dUdx.y;
	const F32 dudy = texSize.x * dUdy.x;
	const F32 dvdy = texSize.y * dUdy.y;

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

// Isotropic LOD calculation taken from the GL spec
F32 computeTextureLodIsotropic(Vec2 texDim, Vec2 ddx, Vec2 ddy, F32 lodBias)
{
	// Derivatives in texel space
	const Vec2 dX = ddx * texDim;
	const Vec2 dY = ddy * texDim;

	// The footprint scale. The max() also avoids log2(0)
	const F32 rho = max(max(length(dX), length(dY)), kEpsilonF32);

	return log2(rho) + lodBias; // May be negative for magnification
}

// ===========================================================================
// Texture 2D                                                                =
// ===========================================================================

// Something like Texture2D::SampleLevel() but the lod is not a float
Vec4 sampleTexture2DLod(ImageDescriptor desc, SamplerState sampl, Vec2 uv, I32 lod)
{
	g_detailedImageLodFeedback = lod;

	const I32 lodi = clamp(lod, I32(desc.m_firstMipmap), I32(desc.m_lastMipmap));

	const U32 packedBindlessIndexAndLod = desc.m_bindlessTextureIndexAndLod[lodi];
	const U32 bindlessIndex = packedBindlessIndexAndLod >> 8u;
	const U32 texLod = packedBindlessIndexAndLod & 0xFFu;

	const Vec4 final = getBindlessTextureNonUniformIndex2DVec4(bindlessIndex).SampleLevel(sampl, uv, texLod);

	return final;
}

// Stochastic aniso texture sampling
// TODO: At the moment sampler's aniso is ignored because this function actualy uses SampleLevel(). Check if you can replace the SampleLevel()
//       with SampleGrad()
Vec4 sampleTexture2DAnisoGrad(ImageDescriptor desc, SamplerState sampl, Vec2 uv, Vec2 ddx, Vec2 ddy, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	F32 lod = computeTextureLodAnisoGL(Vec2(desc.m_width, desc.m_height), ddx, ddy, lodBias, aniso);
	lod = (frac(lod) < randFactor) ? floor(lod) : ceil(lod);
	return sampleTexture2DLod(desc, sampl, uv, lod);
}

// Stochastic aniso texture sampling
Vec4 sampleTexture2DAniso(ImageDescriptor desc, SamplerState sampl, Vec2 uv, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	return sampleTexture2DAnisoGrad(desc, sampl, uv, ddx(uv), ddy(uv), aniso, randFactor, lodBias);
}

// Stochastic isotropic texture sampling
Vec4 sampleTexture2DGrad(ImageDescriptor desc, SamplerState sampl, Vec2 uv, Vec2 ddx, Vec2 ddy, F32 randFactor, F32 lodBias = 0.0)
{
	F32 lod = computeTextureLodIsotropic(Vec2(desc.m_width, desc.m_height), ddx, ddy, lodBias);
	lod = (frac(lod) < randFactor) ? floor(lod) : ceil(lod);
	return sampleTexture2DLod(desc, sampl, uv, lod);
}

// Stochastic isotropic texture sampling
Vec4 sampleTexture2D(ImageDescriptor desc, SamplerState sampl, Vec2 uv, F32 randFactor, F32 lodBias = 0.0)
{
	return sampleTexture2DGrad(desc, sampl, uv, ddx(uv), ddy(uv), randFactor, lodBias);
}

// ===========================================================================
// Texture 2D Array                                                          =
// ===========================================================================

// See sampleTexture2DLod()
Vec4 sampleTexture2DArrayLod(ImageDescriptor desc, SamplerState sampl, Vec3 uvw, I32 lod)
{
	g_detailedImageLodFeedback = lod;

	const I32 lodi = clamp(lod, I32(desc.m_firstMipmap), I32(desc.m_lastMipmap));

	const U32 packedBindlessIndexAndLod = desc.m_bindlessTextureIndexAndLod[lodi];
	const U32 bindlessIndex = packedBindlessIndexAndLod >> 8u;
	const U32 texLod = packedBindlessIndexAndLod & 0xFFu;

	const Vec4 final = getBindlessTextureNonUniformIndex2DArrayVec4(bindlessIndex).SampleLevel(sampl, uvw, texLod);

	return final;
}

// See sampleTexture2DAnisoGrad()
Vec4 sampleTexture2DArrayAnisoGrad(ImageDescriptor desc, SamplerState sampl, Vec3 uvw, Vec2 ddx, Vec2 ddy, F32 aniso, F32 randFactor,
								   F32 lodBias = 0.0)
{
	F32 lod = computeTextureLodAnisoGL(Vec2(desc.m_width, desc.m_height), ddx, ddy, lodBias, aniso);
	lod = (frac(lod) < randFactor) ? floor(lod) : ceil(lod);
	return sampleTexture2DArrayLod(desc, sampl, uvw, lod);
}

// See sampleTexture2DAniso()
Vec4 sampleTexture2DArrayAniso(ImageDescriptor desc, SamplerState sampl, Vec3 uvw, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	return sampleTexture2DArrayAnisoGrad(desc, sampl, uvw, ddx(uvw.xy), ddy(uvw.xy), aniso, randFactor, lodBias);
}

// See sampleTexture2DGrad()
Vec4 sampleTexture2DArrayGrad(ImageDescriptor desc, SamplerState sampl, Vec3 uvw, Vec2 ddx, Vec2 ddy, F32 randFactor, F32 lodBias = 0.0)
{
	F32 lod = computeTextureLodIsotropic(Vec2(desc.m_width, desc.m_height), ddx, ddy, lodBias);
	lod = (frac(lod) < randFactor) ? floor(lod) : ceil(lod);
	return sampleTexture2DArrayLod(desc, sampl, uvw, lod);
}

// See sampleTexture2D()
Vec4 sampleTexture2DArray(ImageDescriptor desc, SamplerState sampl, Vec3 uvw, F32 randFactor, F32 lodBias = 0.0)
{
	return sampleTexture2DArrayGrad(desc, sampl, uvw, ddx(uvw.xy), ddy(uvw.xy), randFactor, lodBias);
}
