// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Spherical Harmonics utilities

#pragma once

#include <AnKi/Shaders/Common.hlsl>

constexpr U32 kSHL1CoefficientCount = (1 + 1) * (1 + 1);
constexpr F32 kSHBasisL0 = 1.0 / (2.0 * sqrt(kPi));
constexpr F32 kSHBasisL1 = sqrt(3.0) / (2.0 * sqrt(kPi));

/// 2 bands, 4 coefficients per color component.
template<typename T>
struct SHL1
{
	vector<T, 3> m_c[kSHL1CoefficientCount];
};

template<typename T>
SHL1<T> appendSH(SHL1<T> inputSH, vector<T, 3> direction, vector<T, 3> value, U32 sampleCount)
{
	SHL1<T> res;

	// L0
	res.m_c[0] = T(kSHBasisL0) * value;

	// L1
	res.m_c[1] = T(kSHBasisL1) * direction.y * value;
	res.m_c[2] = T(kSHBasisL1) * direction.z * value;
	res.m_c[3] = T(kSHBasisL1) * direction.x * value;

	const T weight = T(1) / T(sampleCount);
	[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
	{
		inputSH.m_c[i] += res.m_c[i] * weight;
	}

	return inputSH;
}
