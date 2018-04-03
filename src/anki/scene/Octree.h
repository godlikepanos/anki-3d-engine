// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/collision/Forward.h>
#include <anki/util/WeakArray.h>

namespace anki
{

// Forward
class OctreeLeaf;

/// @addtogroup scene
/// @{

/// XXX
class OctreeElement
{
protected:
	OctreeElement* m_prev = nullptr;
	OctreeElement* m_next = nullptr;
	OctreeLeaf* m_leaf = nullptr;
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
	Vec3 m_aabbMin;
	Vec3 m_aabbMax;
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

	/// Place or re-place an element in the tree.
	/// @note It's thread-safe.
	void placeElement(const Aabb& volume, OctreeElement& element);

	/// Remove an element from the tree.
	/// @note It's thread-safe.
	void removeElement(OctreeElement& element);

private:
	SceneAllocator<U8> m_alloc;

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

	OctreeLeaf& newLeaf();
	void releaseLeaf(OctreeLeaf* leaf);
};
/// @}

} // end namespace anki
