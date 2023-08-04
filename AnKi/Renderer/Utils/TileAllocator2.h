// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// The result of a tile allocation.
enum class TileAllocatorResult2 : U8
{
	kAllocationFailed = 0,
	kAllocationSucceded = 1 << 0, ///< Allocation succedded.
	kTileCached = 1 << 1, ///< The tile was in the cache already. Goes only with kAllocationSucceded.
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TileAllocatorResult2)

/// Allocates tiles out of a tilemap suitable for shadow mapping.
class TileAllocator2
{
public:
	using ArrayOfLightUuids = DynamicArray<U64, MemoryPoolPtrWrapper<StackMemoryPool>>;

	TileAllocator2();

	TileAllocator2(const TileAllocator2&) = delete; // Non-copyable

	~TileAllocator2();

	TileAllocator2& operator=(const TileAllocator2&) = delete; // Non-copyable

	/// Initialize the allocator.
	/// @param tileCountX The size of the smallest tile (0 hierarchy level).
	void init(U32 tileCountX, U32 tileCountY, U32 hierarchyCount, Bool enableCaching);

	/// Allocate some tiles.
	/// @param hierarchy If it's 0 it chooses the smallest tile.
	[[nodiscard]] TileAllocatorResult2 allocate(Timestamp crntTimestamp, U64 lightUuid, U32 hierarchy, Array<U32, 4>& tileViewport,
												ArrayOfLightUuids& kickedOutLightUuids);

	/// Remove an light from the cache.
	void invalidateCache(U64 lightUuid);

private:
	class Tile;

	RendererDynamicArray<Tile> m_allTiles;
	RendererDynamicArray<U32> m_firstTileIdxOfHierarchy;

	RendererHashMap<U64, U32> m_lightUuidToTileIdx;

	U16 m_tileCountX = 0; ///< Tile count for hierarchy 0
	U16 m_tileCountY = 0; ///< Tile count for hierarchy 0
	U8 m_hierarchyCount = 0;
	Bool m_cachingEnabled = false;

	U32 translateTileIdx(U32 x, U32 y, U32 hierarchy) const
	{
		const U32 hierarchyWidth = m_tileCountX >> hierarchy;
		const U32 idx = y * hierarchyWidth + x + m_firstTileIdxOfHierarchy[hierarchy];
		ANKI_ASSERT(idx < m_allTiles.getSize());
		return idx;
	}

	void updateSubTiles(const Tile& updateFrom, U64 crntLightUuid, ArrayOfLightUuids& kickedOutLights);

	void updateSuperTiles(const Tile& updateFrom, U64 crntLightUuid, ArrayOfLightUuids& kickedOutLights);

	/// Given a tile move the hierarchy up and down to update the hierarchy this tile belongs to.
	void updateTileHierarchy(const Tile& updateFrom, U64 crntLightUuid, ArrayOfLightUuids& kickedOutLights)
	{
		updateSubTiles(updateFrom, crntLightUuid, kickedOutLights);
		updateSuperTiles(updateFrom, crntLightUuid, kickedOutLights);
	}

	/// Search for a tile recursively.
	Bool searchTileRecursively(U32 crntTileIdx, U32 crntTileHierarchy, U32 allocationHierarchy, Timestamp crntTimestamp, U32& emptyTileIdx,
							   U32& toKickTileIdx, Timestamp& tileToKickMinTimestamp) const;

	Bool evaluateCandidate(U32 tileIdx, Timestamp crntTimestamp, U32& emptyTileIdx, U32& toKickTileIdx, Timestamp& tileToKickMinTimestamp) const;
};
/// @}

} // end namespace anki
