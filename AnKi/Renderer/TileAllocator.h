// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// The result of a tile allocation.
enum class TileAllocatorResult : U32
{
	kCached, ///< The tile is cached. No need to re-render it.
	kAllocationFailed, ///< No more available tiles.
	kAllocationSucceded ///< Allocation succeded but the tile needs update.
};

/// Allocates tiles out of a tilemap suitable for shadow mapping.
class TileAllocator
{
public:
	TileAllocator() = default;

	TileAllocator(const TileAllocator&) = delete; // Non-copyable

	~TileAllocator();

	TileAllocator& operator=(const TileAllocator&) = delete; // Non-copyable

	/// Initialize the allocator.
	void init(HeapMemoryPool* pool, U32 tileCountX, U32 tileCountY, U32 lodCount, Bool enableCaching);

	/// Allocate some tiles.
	[[nodiscard]] TileAllocatorResult allocate(Timestamp crntTimestamp, Timestamp lightTimestamp, U64 lightUuid,
											   U32 lightFace, U32 drawcallCount, U32 lod, Array<U32, 4>& tileViewport);

	/// Remove an light from the cache.
	void invalidateCache(U64 lightUuid, U32 lightFace);

private:
	class Tile;

	/// A HashMap key.
	class HashMapKey;

	HeapMemoryPool* m_pool = nullptr;
	DynamicArray<Tile> m_allTiles;
	DynamicArray<U32> m_lodFirstTileIndex;

	HashMap<HashMapKey, U32> m_lightInfoToTileIdx;

	U16 m_tileCountX = 0; ///< Tile count for LOD 0
	U16 m_tileCountY = 0; ///< Tile count for LOD 0
	U8 m_lodCount = 0;
	Bool m_cachingEnabled = false;

	U32 translateTileIdx(U32 x, U32 y, U32 lod) const
	{
		const U32 lodWidth = m_tileCountX >> lod;
		const U32 idx = y * lodWidth + x + m_lodFirstTileIndex[lod];
		ANKI_ASSERT(idx < m_allTiles.getSize());
		return idx;
	}

	void updateSubTiles(const Tile& updateFrom);

	void updateSuperTiles(const Tile& updateFrom);

	/// Given a tile move the hierarchy up and down to update the hierarchy this tile belongs to.
	void updateTileHierarchy(const Tile& updateFrom)
	{
		updateSubTiles(updateFrom);
		updateSuperTiles(updateFrom);
	}

	/// Search for a tile recursively.
	Bool searchTileRecursively(U32 crntTileIdx, U32 crntTileLod, U32 allocationLod, Timestamp crntTimestamp,
							   U32& emptyTileIdx, U32& toKickTileIdx, Timestamp& tileToKickMinTimestamp) const;

	Bool evaluateCandidate(U32 tileIdx, Timestamp crntTimestamp, U32& emptyTileIdx, U32& toKickTileIdx,
						   Timestamp& tileToKickMinTimestamp) const;
};
/// @}

} // end namespace anki
