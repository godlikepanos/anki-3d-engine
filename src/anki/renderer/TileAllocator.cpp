// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TileAllocator.h>

namespace anki
{

class TileAllocator::Tile
{
public:
	Timestamp m_lightTimestamp = 0; ///< The last timestamp the light got updated
	Timestamp m_lastUsedTimestamp = 0; ///< The last timestamp this tile was used
	U64 m_lightUuid = 0;
	U32 m_lightDrawcallCount = 0;
	Array<U32, 4> m_viewport = {};
	Array<U32, 4> m_subTiles = {MAX_U32, MAX_U32, MAX_U32, MAX_U32};
	U32 m_superTile = MAX_U32;
	U8 m_lightLod = 0;
	U8 m_lightFace = 0;
};

class TileAllocator::HashMapKey
{
public:
	U64 m_lightUuid;
	U64 m_face;

	U64 computeHash() const
	{
		return anki::computeHash(this, sizeof(*this), 693);
	}
};

TileAllocator::~TileAllocator()
{
	m_lightInfoToTileIdx.destroy(m_alloc);
	m_allTiles.destroy(m_alloc);
	m_lodFirstTileIndex.destroy(m_alloc);
}

void TileAllocator::init(HeapAllocator<U8> alloc, U32 tileCountX, U32 tileCountY, U32 lodCount, Bool enableCaching)
{
	// Preconditions
	ANKI_ASSERT(tileCountX > 0);
	ANKI_ASSERT(tileCountY > 0);
	ANKI_ASSERT(lodCount > 0);

	// Store some stuff
	m_tileCountX = U16(tileCountX);
	m_tileCountY = U16(tileCountY);
	m_lodCount = U8(lodCount);
	m_alloc = alloc;
	m_cachingEnabled = enableCaching;
	m_lodFirstTileIndex.create(m_alloc, lodCount + 1);

	// Create the tile array & index ranges
	U32 tileCount = 0;
	for(U32 lod = 0; lod < lodCount; ++lod)
	{
		const U32 lodTileCountX = tileCountX >> lod;
		const U32 lodTileCountY = tileCountY >> lod;
		ANKI_ASSERT((lodTileCountX << lod) == tileCountX && "Every LOD should be power of 2 of its parent LOD");
		ANKI_ASSERT((lodTileCountY << lod) == tileCountY && "Every LOD should be power of 2 of its parent LOD");

		m_lodFirstTileIndex[lod] = tileCount;

		tileCount += lodTileCountX * lodTileCountY;
	}
	ANKI_ASSERT(tileCount >= tileCountX * tileCountY);
	m_allTiles.create(m_alloc, tileCount);
	m_lodFirstTileIndex[lodCount] = tileCount - 1;

	// Init the tiles
	U32 tileIdx = 0;
	for(U32 lod = 0; lod < lodCount; ++lod)
	{
		const U32 lodTileCountX = tileCountX >> lod;
		const U32 lodTileCountY = tileCountY >> lod;

		for(U32 y = 0; y < lodTileCountY; ++y)
		{
			for(U32 x = 0; x < lodTileCountX; ++x)
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
					for(U32 j = 0; j < 2; ++j)
					{
						for(U32 i = 0; i < 2; ++i)
						{
							const U32 subTileIdx = translateTileIdx((x << 1) + i, (y << 1) + j, lod - 1);
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
	if(updateFrom.m_subTiles[0] == MAX_U32)
	{
		return;
	}

	for(U32 idx : updateFrom.m_subTiles)
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

void TileAllocator::updateSuperTiles(const Tile& updateFrom)
{
	if(updateFrom.m_superTile != MAX_U32)
	{
		m_allTiles[updateFrom.m_superTile].m_lightUuid = 0;
		m_allTiles[updateFrom.m_superTile].m_lastUsedTimestamp = updateFrom.m_lastUsedTimestamp;
		updateSuperTiles(m_allTiles[updateFrom.m_superTile]);
	}
}

Bool TileAllocator::searchTileRecursively(U32 crntTileIdx, U32 crntTileLod, U32 allocationLod, Timestamp crntTimestamp,
										  U32& emptyTileIdx, U32& toKickTileIdx,
										  Timestamp& tileToKickMinTimestamp) const
{
	const Tile& tile = m_allTiles[crntTileIdx];

	if(crntTileLod == allocationLod)
	{
		// We may have a candidate

		const Bool done =
			evaluateCandidate(crntTileIdx, crntTimestamp, emptyTileIdx, toKickTileIdx, tileToKickMinTimestamp);

		if(done)
		{
			return true;
		}
	}
	else if(tile.m_subTiles[0] != MAX_U32)
	{
		// Move down the hierarchy

		ANKI_ASSERT(allocationLod < crntTileLod);

		for(const U32 idx : tile.m_subTiles)
		{
			const Bool done = searchTileRecursively(idx, crntTileLod >> 1, allocationLod, crntTimestamp, emptyTileIdx,
													toKickTileIdx, tileToKickMinTimestamp);

			if(done)
			{
				return true;
			}
		}
	}

	return false;
}

Bool TileAllocator::evaluateCandidate(U32 tileIdx, Timestamp crntTimestamp, U32& emptyTileIdx, U32& toKickTileIdx,
									  Timestamp& tileToKickMinTimestamp) const
{
	const Tile& tile = m_allTiles[tileIdx];

	if(m_cachingEnabled)
	{
		if(tile.m_lastUsedTimestamp == 0)
		{
			// Found empty
			emptyTileIdx = tileIdx;
			return true;
		}
		else if(tile.m_lastUsedTimestamp != crntTimestamp && tile.m_lastUsedTimestamp < tileToKickMinTimestamp)
		{
			// Found one with low timestamp
			toKickTileIdx = tileIdx;
			tileToKickMinTimestamp = tile.m_lightTimestamp;
		}
	}
	else
	{
		if(tile.m_lastUsedTimestamp != crntTimestamp)
		{
			emptyTileIdx = tileIdx;
			return true;
		}
	}

	return false;
}

TileAllocatorResult TileAllocator::allocate(Timestamp crntTimestamp, Timestamp lightTimestamp, U64 lightUuid,
											U32 lightFace, U32 drawcallCount, U32 lod, Array<U32, 4>& tileViewport)
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
	if(m_cachingEnabled)
	{
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

				ANKI_ASSERT(tile.m_lastUsedTimestamp != crntTimestamp
							&& "Trying to allocate the same thing twice in this timestamp?");

				ANKI_ASSERT(tile.m_lightUuid == lightUuid && tile.m_lightLod == lod && tile.m_lightFace == lightFace);

				tileViewport = {tile.m_viewport[0], tile.m_viewport[1], tile.m_viewport[2], tile.m_viewport[3]};

				const Bool needsReRendering =
					tile.m_lightDrawcallCount != drawcallCount || tile.m_lightTimestamp != lightTimestamp;

				tile.m_lightTimestamp = lightTimestamp;
				tile.m_lastUsedTimestamp = crntTimestamp;
				tile.m_lightDrawcallCount = drawcallCount;

				updateTileHierarchy(tile);

				return (needsReRendering) ? TileAllocatorResult::ALLOCATION_SUCCEEDED : TileAllocatorResult::CACHED;
			}
		}
	}

	// Start searching for a suitable tile. Do a hieratchical search to end up with better locality and not better
	// utilization of the atlas' space
	U32 emptyTileIdx = MAX_U32;
	U32 toKickTileIdx = MAX_U32;
	Timestamp tileToKickMinTimestamp = MAX_TIMESTAMP;
	const U32 maxLod = m_lodCount - 1;
	if(lod == maxLod)
	{
		// This search is simple, iterate the tiles of the max LOD

		for(U32 tileIdx = m_lodFirstTileIndex[maxLod]; tileIdx <= m_lodFirstTileIndex[maxLod + 1]; ++tileIdx)
		{
			const Bool done =
				evaluateCandidate(tileIdx, crntTimestamp, emptyTileIdx, toKickTileIdx, tileToKickMinTimestamp);

			if(done)
			{
				break;
			}
		}
	}
	else
	{
		// Need to do a recursive search

		for(U32 tileIdx = m_lodFirstTileIndex[maxLod]; tileIdx <= m_lodFirstTileIndex[maxLod + 1]; ++tileIdx)
		{
			const Bool done = searchTileRecursively(tileIdx, maxLod, lod, crntTimestamp, emptyTileIdx, toKickTileIdx,
													tileToKickMinTimestamp);

			if(done)
			{
				break;
			}
		}
	}

	U32 allocatedTileIdx;
	if(emptyTileIdx != MAX_U32)
	{
		allocatedTileIdx = emptyTileIdx;
	}
	else if(toKickTileIdx != MAX_U32)
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
	allocatedTile.m_lightLod = U8(lod);
	allocatedTile.m_lightFace = U8(lightFace);

	updateTileHierarchy(allocatedTile);

	// Update the cache
	if(m_cachingEnabled)
	{
		m_lightInfoToTileIdx.emplace(m_alloc, key, allocatedTileIdx);
	}

	// Return
	tileViewport = {allocatedTile.m_viewport[0], allocatedTile.m_viewport[1], allocatedTile.m_viewport[2],
					allocatedTile.m_viewport[3]};

	return TileAllocatorResult::ALLOCATION_SUCCEEDED;
}

void TileAllocator::invalidateCache(U64 lightUuid, U32 lightFace)
{
	ANKI_ASSERT(m_cachingEnabled);
	ANKI_ASSERT(lightUuid > 0);

	HashMapKey key;
	key.m_lightUuid = lightUuid;
	key.m_face = lightFace;

	auto it = m_lightInfoToTileIdx.find(key);
	if(it != m_lightInfoToTileIdx.getEnd())
	{
		m_lightInfoToTileIdx.erase(m_alloc, it);
	}
}

} // end namespace anki
