// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#define MAX_CHILDREN 4
#define NUMTHREADS 64

StructuredBuffer<uint> g_initialWork : register(t0);

RWStructuredBuffer<uint> g_finalResult : register(u0);

#define ASSERT(x) \
	do \
	{ \
		if(!(x)) \
		{ \
			InterlockedAdd(g_finalResult[1], 1); \
		} \
	} while(0)

struct FirstNodeInput
{
	uint3 m_svDispatchGrid : SV_DispatchGrid;
};

struct SecondNodeInput
{
	uint3 m_svDispatchGrid : SV_DispatchGrid;

	uint m_workItems[NUMTHREADS];
	uint m_workItemCount;
};

groupshared uint g_newWorkItemCount;

[Shader("node")][NodeLaunch("broadcasting")][NodeIsProgramEntry][NodeMaxDispatchGrid(1, 1, 1)][numthreads(NUMTHREADS, 1, 1)] void
main(DispatchNodeInputRecord<FirstNodeInput> input, uint svDispatchThreadId
	 : SV_DispatchThreadId, uint svGroupIndex : SV_GROUPINDEX, [MaxRecords(MAX_CHILDREN)] NodeOutput<SecondNodeInput> secondNode)
{
	if(svGroupIndex == 0)
	{
		g_newWorkItemCount = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	uint count, stride;
	g_initialWork.GetDimensions(count, stride);
	uint newWorkItemCount = 0;
	uint newWorkItems[MAX_CHILDREN];
	uint firstOutputRecord = 0;

	if(svDispatchThreadId < count)
	{
		const uint workItem = g_initialWork[svDispatchThreadId];

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

			for(uint i = 0; i < MAX_CHILDREN; ++i)
			{
				newWorkItems[i] = newWorkItem;
			}

			InterlockedAdd(g_newWorkItemCount, MAX_CHILDREN, firstOutputRecord);
			newWorkItemCount = MAX_CHILDREN;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	const uint recordCount = (g_newWorkItemCount + NUMTHREADS - 1) / NUMTHREADS;
	GroupNodeOutputRecords<SecondNodeInput> output = secondNode.GetGroupNodeOutputRecords(recordCount);

	if(recordCount)
	{
		for(uint i = 0; i < recordCount; ++i)
		{
			output[i].m_svDispatchGrid = 1;
			const uint begin = i * NUMTHREADS;
			const uint end = min((i + 1) * NUMTHREADS, g_newWorkItemCount);
			output[i].m_workItemCount = end - begin;
		}

		for(uint i = 0; i < newWorkItemCount; ++i)
		{
			const uint k = (firstOutputRecord + i) / NUMTHREADS;
			const uint l = (firstOutputRecord + i) % NUMTHREADS;
			output[k].m_workItems[l] = newWorkItems[i];
		}
	}

	output.OutputComplete();
}

static const int x = 0; // For formatting

[Shader("node")][NodeLaunch("broadcasting")][numthreads(NUMTHREADS, 1, 1)][NodeDispatchGrid(1, 1, 1)][NodeMaxRecursionDepth(16)] void
secondNode(DispatchNodeInputRecord<SecondNodeInput> input, [MaxRecords(MAX_CHILDREN)] NodeOutput<SecondNodeInput> secondNode,
		   uint svGroupIndex : SV_GROUPINDEX)
{
	if(svGroupIndex == 0)
	{
		g_newWorkItemCount = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	uint newWorkItemCount = 0;
	uint newWorkItems[MAX_CHILDREN];
	uint firstOutputRecord = 0;

	if(svGroupIndex < input.Get().m_workItemCount)
	{
		const uint workItem = input.Get().m_workItems[svGroupIndex];

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

			for(uint i = 0; i < MAX_CHILDREN; ++i)
			{
				newWorkItems[i] = newWorkItem;
			}

			InterlockedAdd(g_newWorkItemCount, MAX_CHILDREN, firstOutputRecord);
			newWorkItemCount = MAX_CHILDREN;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	const uint recordCount = (secondNode.IsValid()) ? (g_newWorkItemCount + NUMTHREADS - 1) / NUMTHREADS : 0;
	GroupNodeOutputRecords<SecondNodeInput> output = secondNode.GetGroupNodeOutputRecords(recordCount);

	if(recordCount)
	{
		for(uint i = 0; i < recordCount; ++i)
		{
			output[i].m_svDispatchGrid = 1;
			const uint begin = i * NUMTHREADS;
			const uint end = min((i + 1) * NUMTHREADS, g_newWorkItemCount);
			output[i].m_workItemCount = end - begin;
		}

		for(uint i = 0; i < newWorkItemCount; ++i)
		{
			const uint k = (firstOutputRecord + i) / NUMTHREADS;
			const uint l = (firstOutputRecord + i) % NUMTHREADS;
			output[k].m_workItems[l] = newWorkItems[i];
		}
	}

	output.OutputComplete();

	if(!secondNode.IsValid())
	{
		ASSERT(1);
	}
}
