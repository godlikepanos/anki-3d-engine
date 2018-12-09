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

/// XXX
enum class TileAllocatorResult : U32
{
	CACHED,
	ALLOCATION_FAILED,
	ALLOCATION_SUCCEDDED
};

/// XXX
class TileAllocator : public NonCopyable
{
public:
	~TileAllocator();

	/// XXX
	void init(HeapAllocator<U8> alloc, U32 tileCountX, U32 tileCountY, U32 lodCount);

	/// XXX
	TileAllocatorResult allocate(Timestamp crntTimestamp,
		Timestamp lightTimestamp,
		U64 lightUuid,
		U32 lightFace,
		U32 drawcallCount,
		U32 lod,
		Array<U32, 4>& tileViewport);

private:
	class Tile
	{
	public:
		U64 m_lastUsedTimestamp = 0;
		U64 m_lightUuid = 0;
		U32 m_drawcallCount = 0;
		Array<U32, 4> m_viewport = {};
		Array<U32, 4> m_subTiles = {};
		U32 m_superTile = 0;
		U8 m_lastUsedLod = 0;
		Bool8 m_pinned = false; ///< If true we cannot allocate from it.
	};

	/// A HashMap key.
	class HashMapKey
	{
	public:
		U64 m_lightUuid;
		U64 m_face;

		U64 computeHash() const
		{
			return anki::computeHash(this, sizeof(*this), 693);
		}
	};

	HeapAllocator<U8> m_alloc;
	DynamicArray<Tile> m_allTiles;
	DynamicArray<U32> m_lodFirstTileIndex;

	HashMap<HashMapKey, U32> m_lightInfoToTileIdx;

	U16 m_tileCountX = 0; ///< Tile count for LOD 0
	U16 m_tileCountY = 0; ///< Tile count for LOD 0
	U8 m_lodCount = 0;

	U32 translateTileIdx(U32 x, U32 y, U32 lod) const
	{
		const U lodWidth = m_tileCountX >> lod;
		const U idx = y * lodWidth + x + m_lodFirstTileIndex[lod];
		ANKI_ASSERT(idx < m_allTiles.getSize());
		return idx;
	}

	void updateSubTiles(const Tile& updateFrom);

	void updateSuperTiles(const Tile& updateFrom);
};
/// @}

} // end namespace anki