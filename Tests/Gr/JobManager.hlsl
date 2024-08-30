// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#define LOCK(spinlock) \
	for(uint keepWaiting__ = true; keepWaiting__;) \
	{ \
		uint locked__; \
		InterlockedCompareExchange(spinlock, 0, 1, locked__); \
		if(locked__ == 0) \
		{ \
			locked__ = locked__

#define UNLOCK(spinlock) \
	InterlockedExchange(spinlock, 0, locked__); \
	keepWaiting__ = false; \
	} \
	}

#define NUMTHREADS 64
#define MAX_CHILDREN 4

struct Queue
{
	uint m_spinlock;
	uint m_head;
	uint m_tail;
	uint m_pendingWork;
};

globallycoherent RWStructuredBuffer<Queue> g_queue : register(u0);
globallycoherent RWStructuredBuffer<uint> g_ringBuffer : register(u1);

RWStructuredBuffer<uint> g_finalResult : register(u2);

struct Consts
{
	uint m_ringBufferSize;
	uint m_padding[3];
};

#if defined(__spirv__)
[[vk::push_constant]] ConstantBuffer<Consts> g_consts;
#else
ConstantBuffer<Consts> g_consts : register(b0, space3000);
#endif

groupshared uint g_inWorkItems[NUMTHREADS];
groupshared uint g_inWorkItemCount;
groupshared bool g_bNoMoreWork;

groupshared uint g_outWorkItems[NUMTHREADS * MAX_CHILDREN];
groupshared uint g_outWorkItemCount;

[numthreads(NUMTHREADS, 1, 1)] void main(uint svGroupIndex : SV_GroupIndex)
{
	while(true)
	{
		GroupMemoryBarrierWithGroupSync();

		// Dequeue work
		if(svGroupIndex == 0)
		{
			LOCK(g_queue[0].m_spinlock);

			const uint workItemCount = min(NUMTHREADS, g_queue[0].m_head - g_queue[0].m_tail);

			for(uint it = 0; it < workItemCount; ++it)
			{
				g_inWorkItems[it] = g_ringBuffer[(g_queue[0].m_tail + it) & (g_consts.m_ringBufferSize - 1u)];
			}

			g_inWorkItemCount = workItemCount;
			g_queue[0].m_tail += workItemCount;
			g_queue[0].m_pendingWork += workItemCount;

			g_bNoMoreWork = g_queue[0].m_pendingWork == 0;

			UNLOCK(g_queue[0].m_spinlock);

			g_outWorkItemCount = 0;
		}

		GroupMemoryBarrierWithGroupSync();

		// Do work
		if(g_bNoMoreWork)
		{
			// No more work
			break;
		}
		else if(svGroupIndex < g_inWorkItemCount)
		{
			const uint workItem = g_inWorkItems[svGroupIndex];

			const uint level = workItem >> 16u;
			const uint payload = workItem & 0xFFFFu;

			if(level == 0)
			{
				InterlockedAdd(g_finalResult[0], payload);
			}
			else
			{
				uint newWorkItem = (level - 1) << 16u;
				newWorkItem |= payload;

				uint slot;
				InterlockedAdd(g_outWorkItemCount, MAX_CHILDREN, slot);

				for(uint i = 0; i < MAX_CHILDREN; ++i)
				{
					g_outWorkItems[slot + i] = newWorkItem;
				}
			}
		}

		GroupMemoryBarrierWithGroupSync();

		// Push new work
		if(svGroupIndex == 0)
		{
			bool success = true;
			int iterationCount = 1000;
			do
			{
				LOCK(g_queue[0].m_spinlock);

				const bool full = (g_queue[0].m_head - g_queue[0].m_tail) + g_outWorkItemCount >= g_consts.m_ringBufferSize;
				success = !full;
				if(success)
				{
					for(uint i = 0; i < g_outWorkItemCount; ++i)
					{
						g_ringBuffer[(g_queue[0].m_head + i) & (g_consts.m_ringBufferSize - 1u)] = g_outWorkItems[i];
					}

					g_queue[0].m_head += g_outWorkItemCount;
					g_queue[0].m_pendingWork -= g_inWorkItemCount;
				}

				UNLOCK(g_queue[0].m_spinlock);
			} while(!success && (iterationCount-- > 0));

			if(iterationCount <= 0)
			{
				InterlockedAdd(g_finalResult[1], 1);
			}
		}
	}
}
