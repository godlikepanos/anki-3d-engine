// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/collision/Forward.h>
#include <anki/util/WeakArray.h>
#include <anki/util/Enum.h>

namespace anki
{

// Forward
class OctreeLeaf;

/// @addtogroup scene
/// @{

/// XXX
class OctreeElement
{
public:
	Bool8 m_visited = false;
};

/// XXX
/// @warning Keept its size as small as possible.
/// @warning Watch the members. Should be be able to zero its memory to initialize it.
class OctreeLeaf
{
public:
	OctreeElement* m_first;
	OctreeElement* m_last;
	Array<OctreeLeaf*, 8> m_leafs;
};

/// Octree for visibility tests.
class Octree
{
public:
	Octree(SceneAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~Octree();

	void init(const Vec3& sceneAabbMin, const Vec3& sceneAabbMax, U32 maxDepth);

	/// Place or re-place an element in the tree.
	/// @note It's thread-safe.
	void placeElement(const Aabb& volume, OctreeElement* element);

	/// Remove an element from the tree.
	/// @note It's thread-safe.
	void removeElement(OctreeElement& element);

private:
	SceneAllocator<U8> m_alloc;
	U32 m_maxDepth = 0;
	Vec3 m_sceneAabbMin = Vec3(0.0f);
	Vec3 m_sceneAabbMax = Vec3(0.0f);

	/// Keep the allocations of leafes tight because we want quite alot of them.
	class LeafStorage
	{
	public:
		WeakArray<OctreeLeaf> m_leafs;
		WeakArray<U16> m_unusedLeafsStack;
		U16 m_unusedLeafCount = MAX_U16;
	};

	static constexpr U LEAFES_PER_STORAGE = 256;

	DynamicArray<LeafStorage> m_leafStorages;

	OctreeLeaf* m_rootLeaf = nullptr;

	/// P: Stands for positive and N: Negative
	enum class LeafMask : U8
	{
		PX_PY_PZ = 1 << 0,
		PX_PY_NZ = 1 << 1,
		PX_NY_PZ = 1 << 2,
		PX_NY_NZ = 1 << 3,
		NX_PY_PZ = 1 << 4,
		NX_PY_NZ = 1 << 5,
		NX_NY_PZ = 1 << 6,
		NX_NY_NZ = 1 << 7,

		NONE = 0,
		ALL = PX_PY_PZ | PX_PY_NZ | PX_NY_PZ | PX_NY_NZ | NX_PY_PZ | NX_PY_NZ | NX_NY_PZ | NX_NY_NZ,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(LeafMask, friend)

	OctreeLeaf& newLeaf();
	void releaseLeaf(OctreeLeaf* leaf);

	void placeElementRecursive(
		const Aabb& volume, OctreeElement* element, OctreeLeaf& parent, const Vec3& aabbMin, const Vec3& aabbMax);
};
/// @}

} // end namespace anki
