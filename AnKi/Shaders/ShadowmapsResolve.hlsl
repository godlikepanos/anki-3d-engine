// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/PackFunctions.hlsl>

constexpr U32 kMaxShadowCastersPerFragment = 8u;

template<typename T>
TVec<T, 4> packShadowLayers(T layers[kMaxShadowCastersPerFragment])
{
	TVec<T, 4> packed;
	[unroll] for(U32 i = 0; i < kMaxShadowCastersPerFragment / 2u; ++i)
	{
		packed[i] = packUnorm2ToUnorm1(TVec<T, 2>(layers[i * 2u], layers[i * 2u + 1u]));
	}
	return packed;
}

template<typename T>
T getResolvedShadow(TVec<T, 4> packedShadows, U32 layer)
{
	ANKI_ASSERT(layer < kMaxShadowCastersPerFragment);
#if 0
	const TVec<T, 2> shadowPair = unpackUnorm1ToUnorm2(packedShadows[layer >> 1u]);
	return shadowPair[layer & 1u];
#else
	const T temp = packedShadows[layer >> 1u] * T(255.0 / 16.0);
	const T high = floor(temp);
	return ((layer & 1u) == 0u) ? high * T(1.0 / 15.0) : (temp - high) * T(16.0 / 15.0);
#endif
}

template<typename T>
T getNextResolvedShadow(TVec<T, 4> packedShadows, inout U32 layer)
{
	layer = min(layer, kMaxShadowCastersPerFragment - 1u);
	return getResolvedShadow(packedShadows, layer++);
}
