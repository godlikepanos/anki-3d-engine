// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/*
bool isPrime(uint N)
{
	for(uint j = 2; j < N / 2; ++j)
	{
		if(i % N == 0) return false;
	}
	return true;
}
*/

#define NUMTHREADS 256

#if WORKGRAPHS
struct FirstNodeInput
{
	uint3 m_svDispatchGrid : SV_DispatchGrid;
};
#endif

struct Misc
{
	uint m_threadgroupCounter;
	uint m_threadgroupCount;
	uint m_primeNumberCount;
};

RWStructuredBuffer<uint> g_primeNumbers : register(u0);
RWStructuredBuffer<Misc> g_misc : register(u1);

groupshared uint g_isPrime;

#if WORKGRAPHS
[Shader("node")][NodeLaunch("broadcasting")][NodeIsProgramEntry][NodeMaxDispatchGrid(1, 1, 1)][numthreads(NUMTHREADS, 1, 1)]
#else
[numthreads(NUMTHREADS, 1, 1)]
#endif
	void
	main(uint svGroupIndex : SV_GroupIndex, uint svGroupId : SV_GroupID
#if WORKGRAPHS
		 ,
		 DispatchNodeInputRecord<FirstNodeInput> input
#endif
	)
{
	if(svGroupIndex == 0)
	{
		g_isPrime = 1;
	}
	GroupMemoryBarrierWithGroupSync();

	const uint N = svGroupId; // The candidate number to check

	if(N >= 2)
	{
		// Iterations to find if it's prime
		const uint iterations = N / 2 - 2;

		const uint loopCountPerThread = (iterations + NUMTHREADS - 1) / NUMTHREADS;

		for(uint i = 0; i < loopCountPerThread; ++i)
		{
			const uint j = 2 + loopCountPerThread * i + svGroupIndex;
			if(j <= N / 2 && N % j == 0)
			{
				uint origVal;
				InterlockedExchange(g_isPrime, 0, origVal);
				break;
			}
		}
	}
	else
	{
		uint origVal;
		InterlockedExchange(g_isPrime, 0, origVal);
	}

	GroupMemoryBarrierWithGroupSync();

	if(svGroupIndex == 0 && g_isPrime)
	{
		uint offset;
		InterlockedAdd(g_misc[0].m_primeNumberCount, 1, offset);

		g_primeNumbers[offset + 1] = N;
	}

	GroupMemoryBarrierWithGroupSync();

	if(svGroupIndex == 0)
	{
		uint orig;
		InterlockedAdd(g_misc[0].m_threadgroupCounter, 1, orig);
		const bool lastThreadgroup = (orig + 1u) == g_misc[0].m_threadgroupCount;

		if(lastThreadgroup)
		{
			g_primeNumbers[0] = g_misc[0].m_primeNumberCount;

			g_misc[0].m_primeNumberCount = 0;
			g_misc[0].m_threadgroupCounter = 0;
		}
	}
}
