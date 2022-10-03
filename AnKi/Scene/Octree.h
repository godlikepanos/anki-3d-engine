// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Math.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/ObjectAllocator.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

// Forward
class OctreePlaceable;
class ThreadHive;
class ThreadHiveSemaphore;

/// @addtogroup scene
/// @{

/// Callback to determine if an octree node is visible.
using OctreeNodeVisibilityTestCallback = Bool (*)(void* userData, const Aabb& box);

/// Octree debug drawer.
class OctreeDebugDrawer
{
public:
	virtual void drawCube(const Aabb& box, const Vec4& color) = 0;
};

/// Octree for visibility tests.
class Octree
{
	friend class OctreePlaceable;

public:
	Octree(HeapMemoryPool* pool)
		: m_pool(pool)
	{
	}

	Octree(const Octree&) = delete; // Non-copyable

	~Octree();

	Octree& operator=(const Octree&) = delete; // Non-copyable

	void init(const Vec3& sceneAabbMin, const Vec3& sceneAabbMax, U32 maxDepth);

	/// Place or re-place an element in the tree.
	/// @note It's thread-safe against place and remove methods.
	void place(const Aabb& volume, OctreePlaceable* placeable, Bool updateActualSceneBounds);

	/// Place the placeable somewhere where it's always visible.
	/// @note It's thread-safe against place and remove methods.
	void placeAlwaysVisible(OctreePlaceable* placeable);

	/// Remove an element from the tree.
	/// @note It's thread-safe against place and remove methods.
	void remove(OctreePlaceable& placeable);

	/// Gather visible placeables.
	/// @param frustumPlanes The frustum planes to test against.
	/// @param testId A unique index for this test.
	/// @param testCallback A ptr to a function that will be used to perform an additional test to the box of the
	///                     Octree node. Can be nullptr.
	/// @param testCallbackUserData Parameter to the testCallback. Can be nullptr.
	/// @param out The output of the tests.
	/// @note It's thread-safe against other gatherVisible calls.
	void gatherVisible(const Plane frustumPlanes[6], U32 testId, OctreeNodeVisibilityTestCallback testCallback,
					   void* testCallbackUserData, DynamicArrayRaii<void*>& out)
	{
		gatherVisibleRecursive(frustumPlanes, testId, testCallback, testCallbackUserData, m_rootLeaf, out);
	}

	/// Similar to gatherVisible but it spawns ThreadHive tasks.
	void gatherVisibleParallel(const Plane frustumPlanes[6], U32 testId, OctreeNodeVisibilityTestCallback testCallback,
							   void* testCallbackUserData, DynamicArrayRaii<void*>* out, ThreadHive& hive,
							   ThreadHiveSemaphore* waitSemaphore, ThreadHiveSemaphore*& signalSemaphore);

	/// Walk the tree.
	/// @tparam TTestAabbFunc The lambda that will test an Aabb. Signature of lambda: Bool(*)(const Aabb& leafBox)
	/// @tparam TNewPlaceableFunc The lambda to do something with a visible placeable.
	///                           Signature: void(*)(void* placeableUserData).
	/// @param testId The test index.
	/// @param testFunc See TTestAabbFunc.
	/// @param newPlaceableFunc See TNewPlaceableFunc.
	template<typename TTestAabbFunc, typename TNewPlaceableFunc>
	void walkTree(U32 testId, TTestAabbFunc testFunc, TNewPlaceableFunc newPlaceableFunc)
	{
		ANKI_ASSERT(m_rootLeaf);
		walkTreeInternal(*m_rootLeaf, testId, testFunc, newPlaceableFunc);
	}

	/// Debug draw.
	void debugDraw(OctreeDebugDrawer& drawer) const
	{
		ANKI_ASSERT(m_rootLeaf);
		debugDrawRecursive(*m_rootLeaf, drawer);
	}

	/// Get the bounds of the scene as calculated by the objects that were placed inside the Octree.
	void getActualSceneBounds(Vec3& min, Vec3& max) const
	{
		LockGuard<Mutex> lock(m_globalMtx);
		ANKI_ASSERT(m_actualSceneAabbMin.x() < kMaxF32);
		ANKI_ASSERT(m_actualSceneAabbMax.x() > kMinF32);
		min = m_actualSceneAabbMin;
		max = m_actualSceneAabbMax;
	}

private:
	class GatherParallelCtx;
	class GatherParallelTaskCtx;

	/// List node.
	class PlaceableNode : public IntrusiveListEnabled<PlaceableNode>
	{
	public:
		OctreePlaceable* m_placeable = nullptr;

#if ANKI_ENABLE_ASSERTIONS
		~PlaceableNode()
		{
			m_placeable = nullptr;
		}
#endif
	};

	/// Octree leaf.
	/// @warning Keept its size as small as possible.
	class Leaf
	{
	public:
		IntrusiveList<PlaceableNode> m_placeables;
		Vec3 m_aabbMin;
		Vec3 m_aabbMax;
		Array<Leaf*, 8> m_children = {};

#if ANKI_ENABLE_ASSERTIONS
		~Leaf()
		{
			ANKI_ASSERT(m_placeables.isEmpty());
			m_children = {};
			m_aabbMin = m_aabbMax = Vec3(0.0f);
		}
#endif

		Bool hasChildren() const
		{
			return m_children[0] != nullptr || m_children[1] != nullptr || m_children[2] != nullptr
				   || m_children[3] != nullptr || m_children[4] != nullptr || m_children[5] != nullptr
				   || m_children[6] != nullptr || m_children[7] != nullptr;
		}
	};

	/// Used so that OctreePlaceable knows which leafs it belongs to.
	class LeafNode : public IntrusiveListEnabled<LeafNode>
	{
	public:
		Leaf* m_leaf = nullptr;

#if ANKI_ENABLE_ASSERTIONS
		~LeafNode()
		{
			m_leaf = nullptr;
		}
#endif
	};

	/// Pos: Stands for positive and Neg: Negative
	enum class LeafMask : U8
	{
		kPosXPosYPosZ = 1 << 0,
		kPosXPosYNegZ = 1 << 1,
		kPosXNegYPosZ = 1 << 2,
		kPosXNegYNegZ = 1 << 3,
		kNegXPosYPosZ = 1 << 4,
		kNegXPosYNegZ = 1 << 5,
		kNegXNegYPosZ = 1 << 6,
		kNegXNegYNegZ = 1 << 7,

		kNone = 0,
		kAll = kPosXPosYPosZ | kPosXPosYNegZ | kPosXNegYPosZ | kPosXNegYNegZ | kNegXPosYPosZ | kNegXPosYNegZ
			   | kNegXNegYPosZ | kNegXNegYNegZ,
		kRight = kPosXPosYPosZ | kPosXPosYNegZ | kPosXNegYPosZ | kPosXNegYNegZ,
		kLeft = kNegXPosYPosZ | kNegXPosYNegZ | kNegXNegYPosZ | kNegXNegYNegZ,
		kTop = kPosXPosYPosZ | kPosXPosYNegZ | kNegXPosYPosZ | kNegXPosYNegZ,
		kBottom = kPosXNegYPosZ | kPosXNegYNegZ | kNegXNegYPosZ | kNegXNegYNegZ,
		kFront = kPosXPosYPosZ | kPosXNegYPosZ | kNegXPosYPosZ | kNegXNegYPosZ,
		kBack = kPosXPosYNegZ | kPosXNegYNegZ | kNegXPosYNegZ | kNegXNegYNegZ,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(LeafMask)

	HeapMemoryPool* m_pool;
	U32 m_maxDepth = 0;
	Vec3 m_sceneAabbMin = Vec3(0.0f);
	Vec3 m_sceneAabbMax = Vec3(0.0f);
	mutable Mutex m_globalMtx;

	ObjectAllocatorSameType<Leaf, 256> m_leafAlloc;
	ObjectAllocatorSameType<LeafNode, 128> m_leafNodeAlloc;
	ObjectAllocatorSameType<PlaceableNode, 256> m_placeableNodeAlloc;

	Leaf* m_rootLeaf = nullptr;
	U32 m_placeableCount = 0;

	/// Compute the min of the scene bounds based on what is placed inside the octree.
	Vec3 m_actualSceneAabbMin = Vec3(kMaxF32);
	Vec3 m_actualSceneAabbMax = Vec3(kMinF32);

	Leaf* newLeaf()
	{
		return m_leafAlloc.newInstance(*m_pool);
	}

	void releaseLeaf(Leaf* leaf)
	{
		m_leafAlloc.deleteInstance(*m_pool, leaf);
	}

	PlaceableNode* newPlaceableNode(OctreePlaceable* placeable)
	{
		ANKI_ASSERT(placeable);
		PlaceableNode* out = m_placeableNodeAlloc.newInstance(*m_pool);
		out->m_placeable = placeable;
		return out;
	}

	void releasePlaceableNode(PlaceableNode* placeable)
	{
		m_placeableNodeAlloc.deleteInstance(*m_pool, placeable);
	}

	LeafNode* newLeafNode(Leaf* leaf)
	{
		ANKI_ASSERT(leaf);
		LeafNode* out = m_leafNodeAlloc.newInstance(*m_pool);
		out->m_leaf = leaf;
		return out;
	}

	void releaseLeafNode(LeafNode* node)
	{
		m_leafNodeAlloc.deleteInstance(*m_pool, node);
	}

	void placeRecursive(const Aabb& volume, OctreePlaceable* placeable, Leaf* parent, U32 depth);

	static Bool volumeTotallyInsideLeaf(const Aabb& volume, const Leaf& leaf);

	static void computeChildAabb(LeafMask child, const Vec3& parentAabbMin, const Vec3& parentAabbMax,
								 const Vec3& parentAabbCenter, Vec3& childAabbMin, Vec3& childAabbMax);

	/// Remove a placeable from the tree.
	void removeInternal(OctreePlaceable& placeable);

	static void gatherVisibleRecursive(const Plane frustumPlanes[6], U32 testId,
									   OctreeNodeVisibilityTestCallback testCallback, void* testCallbackUserData,
									   Leaf* leaf, DynamicArrayRaii<void*>& out);

	/// ThreadHive callback.
	static void gatherVisibleTaskCallback(void* ud, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* sem);

	void gatherVisibleParallelTask(U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* sem,
								   GatherParallelTaskCtx& taskCtx);

	/// Remove a leaf.
	void cleanupRecursive(Leaf* leaf, Bool& canDeleteLeafUponReturn);

	/// Cleanup the tree.
	void cleanupInternal();

	/// Debug draw.
	void debugDrawRecursive(const Leaf& leaf, OctreeDebugDrawer& drawer) const;

	template<typename TTestAabbFunc, typename TNewPlaceableFunc>
	void walkTreeInternal(Leaf& leaf, U32 testId, TTestAabbFunc testFunc, TNewPlaceableFunc newPlaceableFunc);
};

/// An entity that can be placed in octrees.
class OctreePlaceable
{
	friend class Octree;

public:
	void* m_userData = nullptr;

	OctreePlaceable() = default;

	OctreePlaceable(const OctreePlaceable&) = delete; // Non-copyable

	OctreePlaceable& operator=(const OctreePlaceable&) = delete; // Non-copyable

	void reset()
	{
		for(Atomic<U64>& mask : m_visitedMasks)
		{
			mask.setNonAtomically(0);
		}
	}

private:
	static constexpr U32 kMaxTests = 128;
	Array<Atomic<U64>, kMaxTests / 64> m_visitedMasks = {0u, 0u};
	IntrusiveList<Octree::LeafNode> m_leafs; ///< A list of leafs this placeable belongs.

	/// Check if already visited.
	/// @note It's thread-safe.
	Bool alreadyVisited(U32 testId)
	{
		ANKI_ASSERT(testId < kMaxTests);
		const U32 group = testId / 64;
		const U64 testMask = U64(1u) << U64(testId % 64);
		const U64 prev = m_visitedMasks[group].fetchOr(testMask);
		return !!(testMask & prev);
	}
};

template<typename TTestAabbFunc, typename TNewPlaceableFunc>
inline void Octree::walkTreeInternal(Leaf& leaf, U32 testId, TTestAabbFunc testFunc, TNewPlaceableFunc newPlaceableFunc)
{
	// Visit the placeables that belong to that leaf
	for(PlaceableNode& placeableNode : leaf.m_placeables)
	{
		if(!placeableNode.m_placeable->alreadyVisited(testId))
		{
			ANKI_ASSERT(placeableNode.m_placeable->m_userData);
			newPlaceableFunc(placeableNode.m_placeable->m_userData);
		}
	}

	Aabb aabb;
	[[maybe_unused]] U visibleLeafs = 0;
	for(Leaf* child : leaf.m_children)
	{
		if(child)
		{
			aabb.setMin(child->m_aabbMin);
			aabb.setMax(child->m_aabbMax);
			if(testFunc(aabb))
			{
				++visibleLeafs;
				walkTreeInternal(*child, testId, testFunc, newPlaceableFunc);
			}
		}
	}

	ANKI_TRACE_INC_COUNTER(OCTREE_VISIBLE_LEAFS, visibleLeafs);
}
/// @}

} // end namespace anki
