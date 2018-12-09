// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TileAllocator.h>

namespace anki
{

void TileAllocator::init(HeapAllocator<U8> alloc, U32 tileCountX, U32 tileCountY, U32 lodCount)
{
	// TODO
}

void TileAllocator::updateSubTiles(const Tile& updateFrom)
{
	for(U32 idx : updateFrom.m_subTiles)
	{
		if(idx != MAX_U32)
		{
			m_allTiles[idx].m_lastUsedTimestamp = updateFrom.m_lastUsedTimestamp;
			m_allTiles[idx].m_lightUuid = updateFrom.m_lightUuid;
			m_allTiles[idx].m_lastUsedLod = updateFrom.m_lastUsedLod;

			updateSubTiles(m_allTiles[idx]);
		}
	}
}

void TileAllocator::updateSuperTiles(const Tile& updateFrom)
{
	if(updateFrom.m_superTile != MAX_U32)
	{
		m_allTiles[updateFrom.m_superTile].m_lightUuid = 0;
		updateSuperTiles(m_allTiles[updateFrom.m_superTile]);
	}
}

TileAllocatorResult TileAllocator::allocate(Timestamp crntTimestamp,
	Timestamp lightTimestamp,
	U64 lightUuid,
	U32 lightFace,
	U32 drawcallCount,
	U32 lod,
	Array<U32, 4>& tileViewport)
{
	// Preconditions
	ANKI_ASSERT(crntTimestamp > 0);
	ANKI_ASSERT(lightUuid != 0);
	ANKI_ASSERT(lightFace < 6);
	ANKI_ASSERT(lod < m_lodCount);

	// 1) Search if it's already cached
	HashMapKey key;
	key.m_lightUuid = lightUuid;
	key.m_face = lightFace;
	auto it = m_lightInfoToTileIdx.find(key);
	if(it != m_lightInfoToTileIdx.getEnd())
	{
		Tile& tile = m_allTiles[*it];

		if(tile.m_lightUuid != lightUuid || tile.m_lastUsedLod != lod)
		{
			// Cache entry is wrong, remove it
			m_lightInfoToTileIdx.erase(m_alloc, it);
		}
		else
		{
			// Same light & lod & face, found the cache entry.

			ANKI_ASSERT(tile.m_lightUuid == lightUuid && tile.m_lastUsedLod == lod);

			tileViewport = tile.m_viewport;

			tile.m_lastUsedTimestamp = crntTimestamp;
			tile.m_drawcallCount = drawcallCount;

			return (tile.m_drawcallCount != drawcallCount) ? TileAllocatorResult::ALLOCATION_SUCCEDDED
														   : TileAllocatorResult::CACHED;
		}
	}

	// Start searching for a tile
	U emptyTileIdx = MAX_U;
	U toKickTileIdx = MAX_U;
	Timestamp tileToKickMinTimestamp = MAX_TIMESTAMP;
	const U firstTileIdx = m_lodFirstTileIndex[lod];
	const U lastTileIdx = m_lodFirstTileIndex[lod + 1];
	for(U tileIdx = firstTileIdx; tileIdx <= lastTileIdx; ++tileIdx)
	{
		const Tile& tile = m_allTiles[tileIdx];

		if(tile.m_pinned)
		{
			continue;
		}

		if(tile.m_lightUuid == 0)
		{
			// Found an empty
			emptyTileIdx = tileIdx;
			break;
		}
		else if(tile.m_lastUsedTimestamp != crntTimestamp && tile.m_lastUsedTimestamp < tileToKickMinTimestamp)
		{
			// Found some with low timestamp
			toKickTileIdx = tileIdx;
			tileToKickMinTimestamp = tile.m_lastUsedTimestamp;
		}
	}

	U allocatedTileIdx;
	if(emptyTileIdx != MAX_U)
	{
		allocatedTileIdx = emptyTileIdx;
	}
	else if(toKickTileIdx != MAX_U)
	{
		allocatedTileIdx = toKickTileIdx;
	}
	else
	{
		// Out of tiles
		return TileAllocatorResult::ALLOCATION_FAILED;
	}

	// Allocation succedded, need to do some bookkeeping

	// Mark the allocated tile
	Tile& allocatedTile = m_allTiles[allocatedTileIdx];
	allocatedTile.m_lastUsedTimestamp = crntTimestamp;
	allocatedTile.m_lightUuid = lightUuid;
	allocatedTile.m_drawcallCount = drawcallCount;

	// Mark all sub-tiles as well
	updateSubTiles(allocatedTile);

	// Mark the super tiles as well
	updateSuperTiles(allocatedTile);

	// Update the cache
	m_lightInfoToTileIdx.emplace(m_alloc, key, allocatedTileIdx);

	return TileAllocatorResult::ALLOCATION_SUCCEDDED;
}

} // end namespace anki