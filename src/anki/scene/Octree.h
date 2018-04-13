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
#include <anki/util/ObjectAllocator.h>
#include <anki/util/List.h>

namespace anki
{

// Forward
class OctreeHandle;

/// @addtogroup scene
/// @{

/// Octree for visibility tests.
class Octree
{
	friend class OctreeHandle;

public:
	Octree(SceneAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~Octree();

	void init(const Vec3& sceneAabbMin, const Vec3& sceneAabbMax, U32 maxDepth);

	/// Place or re-place an element in the tree.
	/// @note It's thread-safe.
	void place(const Aabb& volume, OctreeHandle* handle);

	/// Remove an element from the tree.
	/// @note It's thread-safe.
	void remove(OctreeHandle& handle);

	/// Gather visible handles.
	/// @note It's thread-safe.
	DynamicArray<OctreeHandle*> gatherVisible(GenericMemoryPoolAllocator<U8> alloc, U32 testId);

private:
	/// XXX
	class Handle : public IntrusiveListEnabled<Handle>
	{
	public:
		OctreeHandle* m_handle = nullptr;
	};

	/// XXX
	/// @warning Keept its size as small as possible.
	class Leaf
	{
	public:
		IntrusiveList<Handle> m_handles;
		Array<Leaf*, 8> m_leafs = {};
	};

	/// XXX
	class LeafNode : public IntrusiveListEnabled<LeafNode>
	{
	public:
		Leaf* m_leaf = nullptr;
	};

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
		RIGHT = PX_PY_PZ | PX_PY_NZ | PX_NY_PZ | PX_NY_NZ,
		LEFT = NX_PY_PZ | NX_PY_NZ | NX_NY_PZ | NX_NY_NZ,
		TOP = PX_PY_PZ | PX_PY_NZ | NX_PY_PZ | NX_PY_NZ,
		BOTTOM = PX_NY_PZ | PX_NY_NZ | NX_NY_PZ | NX_NY_NZ,
		FRONT = PX_PY_PZ | PX_NY_PZ | NX_PY_PZ | NX_NY_PZ,
		BACK = PX_PY_NZ | PX_NY_NZ | NX_PY_NZ | NX_NY_NZ,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(LeafMask, friend)

	SceneAllocator<U8> m_alloc;
	U32 m_maxDepth = 0;
	Vec3 m_sceneAabbMin = Vec3(0.0f);
	Vec3 m_sceneAabbMax = Vec3(0.0f);

	ObjectAllocatorSameType<Leaf, 256> m_leafAlloc;
	ObjectAllocatorSameType<LeafNode, 128> m_leafNodeAlloc;
	ObjectAllocatorSameType<Handle, 256> m_handleAlloc;

	Leaf* m_rootLeaf = nullptr;

	Leaf* newLeaf()
	{
		return m_leafAlloc.newInstance(m_alloc);
	}

	void releaseLeaf(Leaf* leaf)
	{
		m_leafAlloc.deleteInstance(m_alloc, leaf);
	}

	Handle* newHandle(OctreeHandle* handle)
	{
		ANKI_ASSERT(handle);
		Handle* out = m_handleAlloc.newInstance(m_alloc);
		out->m_handle = handle;
		return out;
	}

	void releaseHandle(Handle* handle)
	{
		m_handleAlloc.deleteInstance(m_alloc, handle);
	}

	LeafNode* newLeafNode(Leaf* leaf)
	{
		ANKI_ASSERT(leaf);
		LeafNode* out = m_leafNodeAlloc.newInstance(m_alloc);
		out->m_leaf = leaf;
		return out;
	}

	void releaseLeafNode(LeafNode* node)
	{
		m_leafNodeAlloc.deleteInstance(m_alloc, node);
	}

	void placeRecursive(
		const Aabb& volume, OctreeHandle* handle, Leaf* parent, const Vec3& aabbMin, const Vec3& aabbMax, U32 depth);
};

/// XXX
class OctreeHandle
{
	friend class Octree;

public:
	void reset()
	{
		m_visitedMask.set(0);
	}

private:
	Atomic<U64> m_visitedMask = {0u};
	IntrusiveList<Octree::LeafNode> m_leafs; ///< A list of leafs this handle belongs.
};
/// @}

} // end namespace anki
