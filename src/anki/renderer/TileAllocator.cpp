// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TileAllocator.h>

namespace anki
{

TileAllocator::~TileAllocator()
{
	m_lightInfoToTileIdx.destroy(m_alloc);
	m_allTiles.destroy(m_alloc);
}

void TileAllocator::init(HeapAllocator<U8> alloc, U32 tileCountX, U32 tileCountY, U32 lodCount)
{
	// Preconditions
	ANKI_ASSERT(tileCountX > 0);
	ANKI_ASSERT(tileCountY > 0);
	ANKI_ASSERT(lodCount > 0);

	// Store some stuff
	m_tileCountX = tileCountX;
	m_tileCountY = tileCountY;
	m_lodCount = lodCount;
	m_lodFirstTileIndex.create(m_alloc, lodCount + 1);

	// Create the tile array & index ranges
	U tileCount = 0;
	for(U lod = 0; lod < lodCount; ++lod)
	{
		const U lodTileCountX = tileCountX >> lod;
		const U lodTileCountY = tileCountY >> lod;
		ANKI_ASSERT((lodTileCountX << lod) == tileCountX && "Every LOD should be power of 2 of its parent LOD");
		ANKI_ASSERT((lodTileCountY << lod) == tileCountY && "Every LOD should be power of 2 of its parent LOD");

		m_lodFirstTileIndex[lod] = tileCount;

		tileCount += lodTileCountX * lodTileCountY;
	}
	ANKI_ASSERT(tileCount >= tileCountX * tileCountY);
	m_allTiles.create(m_alloc, tileCount);
	m_lodFirstTileIndex[lodCount] = tileCount - 1;

	// Init the tiles
	U tileIdx = 0;
	for(U lod = 0; lod < lodCount; ++lod)
	{
		const U lodTileCountX = tileCountX >> lod;
		const U lodTileCountY = tileCountY >> lod;

		for(U y = 0; y < lodTileCountY; ++y)
		{
			for(U x = 0; x < lodTileCountX; ++x)
			{
				ANKI_ASSERT(tileIdx >= m_lodFirstTileIndex[lod] && tileIdx <= m_lodFirstTileIndex[lod + 1]);
				Tile& tile = m_allTiles[tileIdx];

				tile.m_viewport[0] = x << lod;
				tile.m_viewport[1] = y << lod;
				tile.m_viewport[2] = 1 << lod;
				tile.m_viewport[3] = 1 << lod;

				if(lod > 0)
				{
					// Has sub tiles
					for(U j = 0; j < 2; ++j)
					{
						for(U i = 0; i < 2; ++i)
						{
							const U subTileIdx = translateTileIdx(x / 2 + j, y / 2 + j, lod - 1);
							m_allTiles[subTileIdx].m_superTile = tileIdx;

							tile.m_subTiles[j * 2 + i] = subTileIdx;
						}
					}
				}
				else
				{
					// No sub-tiles
				}

				++tileIdx;
			}
		}
	}
}

void TileAllocator::updateSubTiles(const Tile& updateFrom)
{
	for(U32 idx : updateFrom.m_subTiles)
	{
		if(idx != MAX_U32)
		{
			m_allTiles[idx].m_lightTimestamp = updateFrom.m_lightTimestamp;
			m_allTiles[idx].m_lastUsedTimestamp = updateFrom.m_lastUsedTimestamp;
			m_allTiles[idx].m_lightUuid = updateFrom.m_lightUuid;
			m_allTiles[idx].m_lightDrawcallCount = updateFrom.m_lightDrawcallCount;
			m_allTiles[idx].m_lightLod = updateFrom.m_lightLod;
			m_allTiles[idx].m_lightFace = updateFrom.m_lightFace;

			updateSubTiles(m_allTiles[idx]);
		}
	}
}

void TileAllocator::updateSuperTiles(const Tile& updateFrom)
{
	if(updateFrom.m_superTile != MAX_U32)
	{
		m_allTiles[updateFrom.m_superTile].m_lightUuid = 0;
		m_allTiles[updateFrom.m_superTile].m_lastUsedTimestamp = updateFrom.m_lastUsedTimestamp;
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
	ANKI_ASSERT(lightTimestamp > 0);
	ANKI_ASSERT(lightTimestamp <= crntTimestamp);
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

		if(tile.m_lightUuid != lightUuid || tile.m_lightLod != lod || tile.m_lightFace != lightFace)
		{
			// Cache entry is wrong, remove it
			m_lightInfoToTileIdx.erase(m_alloc, it);
		}
		else
		{
			// Same light & lod & face, found the cache entry.

			ANKI_ASSERT(tile.m_lightUuid == lightUuid && tile.m_lightLod == lod && tile.m_lightFace == lightFace);

			tileViewport = tile.m_viewport;

			const Bool needsReRendering =
				tile.m_lightDrawcallCount != drawcallCount || tile.m_lightTimestamp != lightTimestamp;

			tile.m_lightTimestamp = lightTimestamp;
			tile.m_lastUsedTimestamp = crntTimestamp;
			tile.m_lightDrawcallCount = drawcallCount;

			updateTileHierarchy(tile);

			return (needsReRendering) ? TileAllocatorResult::ALLOCATION_SUCCEDDED : TileAllocatorResult::CACHED;
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

		if(tile.m_lastUsedTimestamp == 0)
		{
			// Found empty
			emptyTileIdx = tileIdx;
		}
		else if(tile.m_lastUsedTimestamp != crntTimestamp && tile.m_lastUsedTimestamp < tileToKickMinTimestamp)
		{
			// Found some with low timestamp
			toKickTileIdx = tileIdx;
			tileToKickMinTimestamp = tile.m_lightTimestamp;
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
	allocatedTile.m_lightTimestamp = lightTimestamp;
	allocatedTile.m_lastUsedTimestamp = crntTimestamp;
	allocatedTile.m_lightUuid = lightUuid;
	allocatedTile.m_lightDrawcallCount = drawcallCount;
	allocatedTile.m_lightLod = lod;
	allocatedTile.m_lightFace = lightFace;

	updateTileHierarchy(allocatedTile);

	// Update the cache
	m_lightInfoToTileIdx.emplace(m_alloc, key, allocatedTileIdx);

	return TileAllocatorResult::ALLOCATION_SUCCEDDED;
}

} // end namespace anki