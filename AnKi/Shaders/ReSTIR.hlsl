// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Shaders/ImportanceSampling.hlsl>

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

// Equation (6). In Algorithm 3 line 8 and Algorithm 4 line 6
template<typename T>
void finalizeReservoirBiased(inout Reservoir<T> r, F32 pHatq)
{
	finalizeReservoir(r, pHatq, (r.m_sampleCount > 0.0) ? (1.0 / r.m_sampleCount) : 0.0);
}

// Algorithm 4 but only for 2 reservoirs
// pHatqr1: It's the p_hat of the 1st reservoir's sample
// pHatqr2: It's the p_hat of the 2nd reservoir's sample
// pHatqs: This is the pHat of the selected candidate
template<typename T>
Reservoir<T> combineReservoirs(Reservoir<T> r1, Reservoir<T> r2, F32 pHatqr1, F32 pHatqr2, out F32 pHatqs, inout RandomGenerator randg)
{
	Reservoir<T> s = (Reservoir<T>)0;
	pHatqs = 0.0; // The p^q(s.y)

	constexpr Bool optimal = true;
	if(!optimal)
	{
		// This is the original clean code
		if(updateReservoir(s, r1.m_sample, pHatqr1 * r1.m_weight * r1.m_sampleCount, randg))
		{
			pHatqs = pHatqr1;
		}
	}
	else
	{
		// This is the optimal version of the above code
		s.m_weightSum = pHatqr1 * r1.m_weight * r1.m_sampleCount;
		s.m_sampleCount = 1.0;
		s.m_sample = r1.m_sample;
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
