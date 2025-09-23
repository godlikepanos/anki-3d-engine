// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#define TEX_SIZE_X 4096u
#define TEX_SIZE_Y 4096u
#define TILE_SIZE_X 32u
#define TILE_SIZE_Y 32u
#define TILE_COUNT_X (TEX_SIZE_X / TILE_SIZE_X)
#define TILE_COUNT_Y (TEX_SIZE_Y / TILE_SIZE_Y)
#define TILE_COUNT (TILE_COUNT_X * TILE_COUNT_Y)

struct FirstNodeInput
{
	uint3 m_svDispatchGrid : SV_DispatchGrid;
};

struct SecondNodeInput
{
	uint3 m_svDispatchGrid : SV_DispatchGrid;
};

Texture2D g_inputTex : register(t0);
RWStructuredBuffer<float4> g_tileMaxColors : register(u0);
RWStructuredBuffer<float4> g_result : register(u1);
RWStructuredBuffer<uint> g_threadgroupCount : register(u2);

groupshared float4 g_tileMax[TILE_SIZE_X * TILE_SIZE_Y];

[Shader("node")][NodeLaunch("broadcasting")][NodeIsProgramEntry][NodeMaxDispatchGrid(1, 1, 1)][numthreads(TILE_SIZE_X, TILE_SIZE_Y, 1)] void
main(DispatchNodeInputRecord<FirstNodeInput> input, [MaxRecords(1)] NodeOutput<SecondNodeInput> secondNode,
	 uint2 svDispatchThreadId : SV_DISPATCHTHREADID, uint svGroupIndex : SV_GROUPINDEX, uint2 svGroupId : SV_GROUPID)
{
	g_tileMax[svGroupIndex] = g_inputTex[svDispatchThreadId];

	GroupMemoryBarrierWithGroupSync();

	[loop] for(uint s = TILE_SIZE_X * TILE_SIZE_Y / 2u; s > 0u; s >>= 1u)
	{
		if(svGroupIndex < s)
		{
			g_tileMax[svGroupIndex] = max(g_tileMax[svGroupIndex], g_tileMax[svGroupIndex + s]);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	const uint tileIdx = svGroupId.y * TILE_COUNT_X + svGroupId.x;
	g_tileMaxColors[tileIdx] = g_tileMax[0];

	GroupMemoryBarrierWithGroupSync();

	// Check if it's the last threadgroup executing
	bool lastThreadgroup = false;
	if(svGroupIndex == 0)
	{
		uint orig;
		InterlockedAdd(g_threadgroupCount[0], 1, orig);
		lastThreadgroup = (orig + 1u) == TILE_COUNT;

		if(lastThreadgroup)
		{
			g_threadgroupCount[0] = 0;
		}
	}

	// Submit (or not) new work
	GroupNodeOutputRecords<SecondNodeInput> recs = secondNode.GetGroupNodeOutputRecords(lastThreadgroup ? 1 : 0);

	if(lastThreadgroup)
	{
		recs.Get().m_svDispatchGrid = uint3(64, 1, 1);
	}

	recs.OutputComplete();
}

groupshared float4 g_maxColor[64];

[Shader("node")][NodeLaunch("broadcasting")][NodeMaxDispatchGrid(1, 1, 1)][numthreads(64, 1, 1)] void
secondNode(DispatchNodeInputRecord<SecondNodeInput> inp, uint svGroupIndex : SV_GROUPINDEX)
{
	const uint tilesPerThread = TILE_COUNT / 64;

	const uint start = svGroupIndex * tilesPerThread;
	const uint end = start + tilesPerThread;

	float4 localMax = 0.0;
	for(uint tileIdx = start; tileIdx < end; ++tileIdx)
	{
		localMax = max(localMax, g_tileMaxColors[tileIdx]);
	}

	g_maxColor[svGroupIndex] = localMax;

	GroupMemoryBarrierWithGroupSync();

	[loop] for(uint s = 64 / 2u; s > 0u; s >>= 1u)
	{
		if(svGroupIndex < s)
		{
			g_maxColor[svGroupIndex] = max(g_maxColor[svGroupIndex], g_maxColor[svGroupIndex + s]);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	g_result[0] = g_maxColor[0];
}
