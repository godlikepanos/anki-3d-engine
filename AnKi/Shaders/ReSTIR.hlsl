// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/ImportanceSampling.hlsl>

// ===========================================================================
// Random                                                                    =
// ===========================================================================

struct RandomGenerator
{
	U32 m_state;
};

RandomGenerator createRandomGenerator(UVec2 pixelCoords, U32 frame)
{
	RandomGenerator r;
	r.m_state = hashPcg(pixelCoords.x + hashPcg(pixelCoords.y + hashPcg(frame)));
	return r;
}

// Returns a value in [0, 1). Uses 24 bits so the result can never round up to exactly 1.0
F32 rand(inout RandomGenerator r)
{
	r.m_state = hashPcg(r.m_state);
	return F32(r.m_state >> 8u) / 16777216.0;
}

// ===========================================================================
// Reservoir                                                                 =
// ===========================================================================

template<typename T>
struct Reservoir
{
	T m_sample; // The y
	F32 m_weightSum; // The w_sum
	F32 m_sampleCount; // The M
	F32 m_weight; // The W
};

// Algorithm 2, lines 5 to 9
template<typename T>
Bool updateReservoir(inout Reservoir<T> r, T sample, F32 risWeight, inout RandomGenerator randg)
{
	r.m_weightSum += risWeight;
	r.m_sampleCount += 1.0;

	const Bool accepted = rand(randg) * r.m_weightSum < risWeight; // Slightly changed to avoid the division
	if(accepted)
	{
		r.m_sample = sample;
	}

	return accepted;
}

// Equation (20). misWeight is the m(x_z): the weight the RIS sum is scaled by. It must be the same m() that sums to 1 over all candidates that could
// have produced r.m_sample, otherwise the estimator is biased
template<typename T>
void finalizeReservoir(inout Reservoir<T> r, F32 pHatq, F32 misWeight)
{
	ANKI_ASSERT(pHatq >= 0.0 && misWeight >= 0.0);
	r.m_weight = (pHatq > 0.0) ? (misWeight * r.m_weightSum / pHatq) : 0.0;
}

// This is Algorithm 3 line 8 and Algorithm 4 line 6
template<typename T>
void finalizeReservoirBiased(inout Reservoir<T> r, F32 pHatq)
{
	finalizeReservoir(r, pHatq, (r.m_sampleCount > 0.0) ? (1.0 / r.m_sampleCount) : 0.0);
}

// Algorithm 4 but only for 2 reservoirs
template<typename T>
Reservoir<T> combineReservoirs(Reservoir<T> r1, Reservoir<T> r2, F32 pHatqr1, F32 pHatqr2, inout RandomGenerator randg)
{
	Reservoir<T> s = (Reservoir<T>)0;
	F32 pHatqs = 0.0; // The p^q(s.y)

	if(updateReservoir(s, r1.m_sample, pHatqr1 * r1.m_weight * r1.m_sampleCount, randg))
	{
		pHatqs = pHatqr1;
	}

	if(updateReservoir(s, r2.m_sample, pHatqr2 * r2.m_weight * r2.m_sampleCount, randg))
	{
		pHatqs = pHatqr2;
	}

	s.m_sampleCount = r1.m_sampleCount + r2.m_sampleCount;
	finalizeReservoirBiased(s, pHatqs);

	return s;
}
