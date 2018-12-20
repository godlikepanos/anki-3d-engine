// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>

namespace anki
{

/// @addtogroup renderer
/// @{

/// The result of a tile allocation.
enum class TileAllocatorResult : U32
{
	CACHED, ///< The tile is caches. No need to do anything.
	ALLOCATION_FAILED, ///< No more available tiles.
	ALLOCATION_SUCCEDDED ///< Allocation succeded but the tile needs update.
};

/// Allocates tiles out of a tilemap suitable for shadow mapping.
class TileAllocator : public NonCopyable
{
public:
	~TileAllocator();

	/// Initialize the allocator.
	void init(HeapAllocator<U8> alloc, U32 tileCountX, U32 tileCountY, U32 lodCount, Bool enableCaching);

	/// Allocate some tiles.
	ANKI_USE_RESULT TileAllocatorResult allocate(Timestamp crntTimestamp,
		Timestamp lightTimestamp,
		U64 lightUuid,
		U32 lightFace,
		U32 drawcallCount,
		U32 lod,
		Array<U32, 4>& tileViewport);

	/// Remove an light from the cache.
	void invalidateCache(U64 lightUuid, U32 lightFace);

private:
	class Tile;

	/// A HashMap key.
	class HashMapKey;

	HeapAllocator<U8> m_alloc;
	DynamicArray<Tile> m_allTiles;
	DynamicArray<U32> m_lodFirstTileIndex;

	HashMap<HashMapKey, U32> m_lightInfoToTileIdx;

	U16 m_tileCountX = 0; ///< Tile count for LOD 0
	U16 m_tileCountY = 0; ///< Tile count for LOD 0
	U8 m_lodCount = 0;
	Bool8 m_cachingEnabled = false;

	U32 translateTileIdx(U32 x, U32 y, U32 lod) const
	{
		const U lodWidth = m_tileCountX >> lod;
		const U idx = y * lodWidth + x + m_lodFirstTileIndex[lod];
		ANKI_ASSERT(idx < m_allTiles.getSize());
		return idx;
	}

	void updateSubTiles(const Tile& updateFrom);

	void updateSuperTiles(const Tile& updateFrom);

	void updateTileHierarchy(const Tile& updateFrom)
	{
		updateSubTiles(updateFrom);
		updateSuperTiles(updateFrom);
	}
};
/// @}

} // end namespace anki