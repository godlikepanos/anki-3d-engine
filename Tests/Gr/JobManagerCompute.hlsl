// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#define LOCK(spinlock) \
	for(bool keepWaiting__ = true; keepWaiting__;) \
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

#define NUMTHREADS 128
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
	uint m_ringBufferSizeMinusOne;
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

static const int kMashPushTries = 1000;

[numthreads(NUMTHREADS, 1, 1)] void main(uint svGroupIndex : SV_GroupIndex)
{
	if(svGroupIndex == 0)
	{
		g_inWorkItemCount = 0;
		g_outWorkItemCount = 0;
	}

	while(true)
	{
		GroupMemoryBarrierWithGroupSync();

		if(svGroupIndex == 0)
		{
			bool pushSuccessful = true;
			int iterationCount = kMashPushTries;
			const uint oldInWorkItemCount = g_inWorkItemCount;
			const uint outWorkItemCount = g_outWorkItemCount;
			do
			{
				LOCK(g_queue[0].m_spinlock);

				// Touch groupshared as little as possible
				uint head = g_queue[0].m_head;
				uint tail = g_queue[0].m_tail;
				uint pendingWork = g_queue[0].m_pendingWork;

				// Dequeue work
				if(iterationCount == kMashPushTries)
				{
					const uint workItemCount = min(NUMTHREADS, head - tail);

					for(uint it = 0; it < workItemCount; ++it)
					{
						g_inWorkItems[it] = g_ringBuffer[(tail + it) & g_consts.m_ringBufferSizeMinusOne];
					}

					pendingWork += workItemCount;
					g_inWorkItemCount = workItemCount;
					tail += workItemCount;
				}

				// Push work
				if(outWorkItemCount > 0)
				{
					const bool full = (head - tail) + outWorkItemCount >= (g_consts.m_ringBufferSizeMinusOne + 1);
					pushSuccessful = !full;
					if(pushSuccessful)
					{
						for(uint i = 0; i < outWorkItemCount; ++i)
						{
							g_ringBuffer[(head + i) & g_consts.m_ringBufferSizeMinusOne] = g_outWorkItems[i];
						}

						head += outWorkItemCount;
						g_outWorkItemCount = 0;
					}
				}

				if(pushSuccessful)
				{
					pendingWork -= oldInWorkItemCount;
					g_bNoMoreWork = pendingWork == 0;
				}

				// Restore mem
				g_queue[0].m_head = head;
				g_queue[0].m_tail = tail;
				g_queue[0].m_pendingWork = pendingWork;

				UNLOCK(g_queue[0].m_spinlock);
			} while(!pushSuccessful && (iterationCount-- > 0));

			if(!pushSuccessful)
			{
				InterlockedAdd(g_finalResult[1], 1);
				g_bNoMoreWork = true;
			}
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
	}
}
