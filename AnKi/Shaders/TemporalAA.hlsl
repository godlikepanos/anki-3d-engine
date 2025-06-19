// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions useful for temporal AA like accumulations

#pragma once

#include <AnKi/Shaders/Common.hlsl>

/// Accumualte color using texture gather.
template<typename T, typename TPostProcessTextureFunc>
void accumulateSourceColor(Texture2D<Vec4> tex, SamplerState linearAnyClampSampler, Vec2 newUv, Vec4 texelWeights, TPostProcessTextureFunc func,
						   inout vector<T, 3> m1, inout vector<T, 3> m2, inout vector<T, 3> sourceSample, inout vector<T, 3> neighboorMin,
						   inout vector<T, 3> neighboorMax)
{
	const vector<T, 4> red = tex.GatherRed(linearAnyClampSampler, newUv);
	const vector<T, 4> green = tex.GatherGreen(linearAnyClampSampler, newUv);
	const vector<T, 4> blue = tex.GatherBlue(linearAnyClampSampler, newUv);

	[unroll] for(U32 c = 0; c < 4; ++c)
	{
		if(texelWeights[c] > 0.0)
		{
			const vector<T, 3> neighbor = func(vector<T, 3>(red[c], green[c], blue[c]));

			sourceSample += neighbor * texelWeights[c];

			neighboorMin = min(neighboorMin, neighbor);
			neighboorMax = max(neighboorMax, neighbor);

			m1 += neighbor;
			m2 += neighbor * neighbor;
		}
	}
}

/// Accumualte color using plain texture sampling.
template<typename T, typename TPostProcessTextureFunc>
void accumulateSourceColor(Texture2D<Vec4> tex, Vec2 coord, Vec2 textureSize, T weight, TPostProcessTextureFunc func, inout vector<T, 3> m1,
						   inout vector<T, 3> m2, inout vector<T, 3> sourceSample, inout vector<T, 3> neighboorMin, inout vector<T, 3> neighboorMax)
{
	coord = clamp(coord, 0.0, textureSize - 1.0);

	const vector<T, 3> neighbor = func(TEX(tex, coord).xyz);

	sourceSample += neighbor * weight;

	neighboorMin = min(neighboorMin, neighbor);
	neighboorMax = max(neighboorMax, neighbor);

	m1 += neighbor;
	m2 += neighbor * neighbor;
}

template<typename T>
struct DefaultPostProcessTextureFunc
{
	vector<T, 3> operator()(vector<T, 3> inp)
	{
		return inp;
	}
};

/// Compute the source color and a few other things like moments for temporal AA.
template<typename T, typename TPostProcessTextureFunc = DefaultPostProcessTextureFunc<T> >
void computeSourceColor(Texture2D<Vec4> tex, SamplerState linearAnyClampSampler, Vec2 coord, out vector<T, 3> m1, out vector<T, 3> m2,
						out vector<T, 3> sourceSample, out vector<T, 3> neighboorMin, out vector<T, 3> neighboorMax,
						TPostProcessTextureFunc func = (DefaultPostProcessTextureFunc<T>)0)
{
	sourceSample = 0.0;
	neighboorMin = kMaxF32;
	neighboorMax = kMinF32;
	m1 = 0.0;
	m2 = 0.0;

	Vec2 textureSize;
	tex.GetDimensions(textureSize.x, textureSize.y);

	const Vec2 uv = (coord + 0.5) / textureSize;

	const Vec2 texelSize = 1.0 / textureSize;
	const Vec2 halfTexelSize = texelSize / 2.0;

	// Positioning mentioned bellow is in screen space (bottom left is in the bottom left of the screen)
	// Alogithm wants to sample 9 taps of this:
	// +-+-+-+
	// |6|7|8|
	// +-+-+-+
	// |3|4|5|
	// +-+-+-+
	// |0|1|2|
	// +-+-+-+
	// "uv" points to the middle of 4

	// Bottom left (0, 1, 4, 3)
	Vec2 newUv = uv + Vec2(-halfTexelSize.x, +halfTexelSize.y);
	accumulateSourceColor(tex, linearAnyClampSampler, newUv, vector<T, 4>(0.5, 0.5, 1.0, 0.5), func, m1, m2, sourceSample, neighboorMin,
						  neighboorMax);

	// Top right (4, 5, 8, 7)
	newUv = uv + Vec2(+halfTexelSize.x, -halfTexelSize.y);
	accumulateSourceColor(tex, linearAnyClampSampler, newUv, vector<T, 4>(0.0, 0.5, 0.5, 0.5), func, m1, m2, sourceSample, neighboorMin,
						  neighboorMax);

	// Top left
	accumulateSourceColor(tex, coord + IVec2(-1, -1), textureSize, T(0.5), func, m1, m2, sourceSample, neighboorMin, neighboorMax);

	// Bottom right
	accumulateSourceColor(tex, coord + IVec2(+1, +1), textureSize, T(0.5), func, m1, m2, sourceSample, neighboorMin, neighboorMax);

	// Misc
	sourceSample /= T(1.0 + 0.5 * 8.0);
}

template<typename T, typename TPostProcessTextureFunc = DefaultPostProcessTextureFunc<T> >
vector<T, 3> computeTemporalAA(Texture2D<Vec4> currentTex, SamplerState linearAnyClampSampler, Vec3 history, Vec2 coord,
							   F32 temporalSourceWeight = 0.01, F32 temporalGamma = 1.0,
							   TPostProcessTextureFunc func = (DefaultPostProcessTextureFunc<T>)0)
{
	vector<T, 3> m1;
	vector<T, 3> m2;
	vector<T, 3> sourceSample;
	vector<T, 3> neighboorMin;
	vector<T, 3> neighboorMax;
	computeSourceColor(currentTex, linearAnyClampSampler, coord, m1, m2, sourceSample, neighboorMin, neighboorMax);

	const T sampleCount = 9.0;
	const vector<T, 3> mu = m1 / sampleCount;
	const vector<T, 3> sigma = sqrt(abs((m2 / sampleCount) - (mu * mu)));
	const vector<T, 3> minc = mu - temporalGamma * sigma;
	const vector<T, 3> maxc = mu + temporalGamma * sigma;

	history = clamp(history, minc, maxc);

	// Blend history and current
	const vector<T, 3> compressedSource = sourceSample * rcp(max3(sourceSample) + T(1));
	const vector<T, 3> compressedHistory = history * rcp(max3(history) + T(1));
	const T luminanceSource = computeLuminance(compressedSource);
	const T luminanceHistory = computeLuminance(compressedHistory);

	T sourceWeight = temporalSourceWeight;
	T historyWeight = T(1) - sourceWeight;
	sourceWeight *= T(1) / (T(1) + luminanceSource);
	historyWeight *= T(1) / (T(1) + luminanceHistory);

	const vector<T, 3> finalVal = (sourceSample * sourceWeight + history * historyWeight) / max(sourceWeight + historyWeight, T(0.00001));
	return finalVal;
}
