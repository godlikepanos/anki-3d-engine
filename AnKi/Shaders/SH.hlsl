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
constexpr F32 kSHCosineA0 = kPi;
constexpr F32 kSHCosineA1 = (2.0 * kPi) / 3.0;
constexpr F32 kSHCosineA2 = (0.25 * kPi);

/// 2 bands, 4 coefficients per color component.
template<typename T>
struct SHL1
{
	vector<T, 3> m_c[kSHL1CoefficientCount];
};

template<typename T>
SHL1<T> projectOntoL1(vector<T, 3> direction, vector<T, 3> value)
{
	SHL1<T> res;

	// L0
	res.m_c[0] = T(kSHBasisL0) * value;

	// L1
	res.m_c[1] = T(kSHBasisL1) * direction.y * value;
	res.m_c[2] = T(kSHBasisL1) * direction.z * value;
	res.m_c[3] = T(kSHBasisL1) * direction.x * value;

	return res;
}

template<typename T>
SHL1<T> appendSH(SHL1<T> inputSH, vector<T, 3> direction, vector<T, 3> radiance, U32 sampleCount)
{
	const SHL1<T> res = projectOntoL1<T>(direction, radiance);
	const T spherePDF = T(1) / T(kPi * 4.0);
	const T weight = T(1) / (T(sampleCount) * spherePDF);

	[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
	{
		inputSH.m_c[i] += res.m_c[i] * weight;
	}

	return inputSH;
}

template<typename T>
vector<T, 3> evaluateSH(SHL1<T> sh, vector<T, 3> direction)
{
	vector<T, 3> res = T(0);

	if(true)
	{
		// L0
		res += sh.m_c[0];

		// L1
		res += sh.m_c[1] * direction.y;
		res += sh.m_c[2] * direction.z;
		res += sh.m_c[3] * direction.x;
	}
	else
	{
		SHL1<T> convolved;
		convolved.m_c[0] = sh.m_c[0] * kSHCosineA0;
		convolved.m_c[1] = sh.m_c[1] * kSHCosineA1;
		convolved.m_c[2] = sh.m_c[2] * kSHCosineA1;
		convolved.m_c[3] = sh.m_c[3] * kSHCosineA1;

		const SHL1<T> projectedDelta = projectOntoL1<T>(direction, T(1));

		[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
		{
			res += convolved.m_c[i] * projectedDelta.m_c[i];
		}
	}

	return res;
}

template<typename T>
SHL1<T> lerpSH(SHL1<T> a, SHL1<T> b, T f)
{
	[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
	{
		a.m_c[i] = lerp(a.m_c[i], b.m_c[i], f);
	}

	return a;
}

template<typename T>
SHL1<T> loadSH(Texture3D<Vec4> red, Texture3D<Vec4> green, Texture3D<Vec4> blue, SamplerState sampler, Vec3 uvw)
{
	vector<T, 4> colorComp[3];
	colorComp[0] = red.SampleLevel(sampler, uvw, 0.0);
	colorComp[1] = green.SampleLevel(sampler, uvw, 0.0);
	colorComp[2] = blue.SampleLevel(sampler, uvw, 0.0);

	SHL1<T> sh;
	[unroll] for(U32 comp = 0; comp < 3; ++comp)
	{
		[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
		{
			sh.m_c[i][comp] = colorComp[comp][i];
		}
	}

	return sh;
}

template<typename T>
SHL1<T> loadSH(Texture3D<Vec4> red, Texture3D<Vec4> green, Texture3D<Vec4> blue, UVec3 coords)
{
	vector<T, 4> colorComp[3];
	colorComp[0] = red[coords];
	colorComp[1] = green[coords];
	colorComp[2] = blue[coords];

	SHL1<T> sh;
	[unroll] for(U32 comp = 0; comp < 3; ++comp)
	{
		[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
		{
			sh.m_c[i][comp] = colorComp[comp][i];
		}
	}

	return sh;
}

template<typename T>
SHL1<T> loadSH(RWTexture3D<Vec4> red, RWTexture3D<Vec4> green, RWTexture3D<Vec4> blue, UVec3 coords)
{
	vector<T, 4> colorComp[3];
	colorComp[0] = red[coords];
	colorComp[1] = green[coords];
	colorComp[2] = blue[coords];

	SHL1<T> sh;
	[unroll] for(U32 comp = 0; comp < 3; ++comp)
	{
		[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
		{
			sh.m_c[i][comp] = colorComp[comp][i];
		}
	}

	return sh;
}

template<typename T>
void storeSH(SHL1<T> sh, RWTexture3D<Vec4> red, RWTexture3D<Vec4> green, RWTexture3D<Vec4> blue, UVec3 coords)
{
	vector<T, 4> colorComp[3];

	[unroll] for(U32 comp = 0; comp < 3; ++comp)
	{
		[unroll] for(U32 i = 0; i < kSHL1CoefficientCount; ++i)
		{
			colorComp[comp][i] = sh.m_c[i][comp];
		}
	}

	red[coords] = colorComp[0];
	green[coords] = colorComp[1];
	blue[coords] = colorComp[2];
}
