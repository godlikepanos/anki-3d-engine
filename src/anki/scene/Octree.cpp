// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Octree.h>
#include <anki/collision/Tests.h>
#include <anki/collision/Aabb.h>
#include <anki/collision/Frustum.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

/// Return a heatmap color.
static Vec3 heatmap(F32 factor)
{
	F32 intPart;
	const F32 fractional = modf(factor * 4.0f, intPart);

	if(intPart < 1.0)
	{
		return mix(Vec3(0.0, 0.0, 0.0), Vec3(0.0, 0.0, 1.0), fractional);
	}
	else if(intPart < 2.0)
	{
		return mix(Vec3(0.0, 0.0, 1.0), Vec3(0.0, 1.0, 0.0), fractional);
	}
	else if(intPart < 3.0)
	{
		return mix(Vec3(0.0, 1.0, 0.0), Vec3(1.0, 1.0, 0.0), fractional);
	}
	else
	{
		return mix(Vec3(1.0, 1.0, 0.0), Vec3(1.0, 0.0, 0.0), fractional);
	}
}

class Octree::GatherParallelCtx
{
public:
	Octree* m_octree = nullptr;
	SpinLock m_lock;
	const Frustum* m_frustum = nullptr;
	U32 m_testId = MAX_U32;
	OctreeNodeVisibilityTestCallback m_testCallback = nullptr;
	void* m_testCallbackUserData = nullptr;
	DynamicArrayAuto<void*>* m_out = nullptr;
};

class Octree::GatherParallelTaskCtx
{
public:
	GatherParallelCtx* m_ctx = nullptr;
	Leaf* m_leaf = nullptr;
};

Octree::~Octree()
{
	ANKI_ASSERT(m_placeableCount == 0);
	cleanupInternal();
	ANKI_ASSERT(m_rootLeaf == nullptr);
}

void Octree::init(const Vec3& sceneAabbMin, const Vec3& sceneAabbMax, U32 maxDepth)
{
	ANKI_ASSERT(sceneAabbMin < sceneAabbMax);
	ANKI_ASSERT(maxDepth > 0);

	m_maxDepth = maxDepth;
	m_sceneAabbMin = sceneAabbMin;
	m_sceneAabbMax = sceneAabbMax;
}

void Octree::place(const Aabb& volume, OctreePlaceable* placeable)
{
	ANKI_ASSERT(placeable);
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(m_sceneAabbMin, m_sceneAabbMax)) && "volume is outside the scene");

	LockGuard<Mutex> lock(m_globalMtx);

	// Remove the placeable from the Octree
	removeInternal(*placeable);

	// Create the root leaf
	if(!m_rootLeaf)
	{
		m_rootLeaf = newLeaf();
		m_rootLeaf->m_aabbMin = m_sceneAabbMin;
		m_rootLeaf->m_aabbMax = m_sceneAabbMax;
	}

	// And re-place it
	placeRecursive(volume, placeable, m_rootLeaf, 0);
	++m_placeableCount;
}

void Octree::remove(OctreePlaceable& placeable)
{
	LockGuard<Mutex> lock(m_globalMtx);
	removeInternal(placeable);
}

Bool Octree::volumeTotallyInsideLeaf(const Aabb& volume, const Leaf& leaf)
{
	const Vec4& amin = volume.getMin();
	const Vec4& amax = volume.getMax();
	const Vec3& bmin = leaf.m_aabbMin;
	const Vec3& bmax = leaf.m_aabbMax;

	Bool superset = true;
	superset = superset && amin.x() <= bmin.x();
	superset = superset && amax.x() >= bmax.x();
	superset = superset && amin.y() <= bmin.y();
	superset = superset && amax.y() >= bmax.y();
	superset = superset && amin.z() <= bmin.z();
	superset = superset && amax.z() >= bmax.z();

	return superset;
}

void Octree::placeRecursive(const Aabb& volume, OctreePlaceable* placeable, Leaf* parent, U32 depth)
{
	ANKI_ASSERT(placeable);
	ANKI_ASSERT(parent);
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(parent->m_aabbMin, parent->m_aabbMax)) && "Should be inside");

	if(depth == m_maxDepth || volumeTotallyInsideLeaf(volume, *parent))
	{
		// Need to stop and bin the placeable to the leaf

		// Checks
#if ANKI_ASSERTS_ENABLED
		for(const LeafNode& node : placeable->m_leafs)
		{
			ANKI_ASSERT(node.m_leaf != parent && "Already binned. That's wrong");
		}

		for(const PlaceableNode& node : parent->m_placeables)
		{
			ANKI_ASSERT(node.m_placeable != placeable);
		}
#endif

		// Connect placeable and leaf
		placeable->m_leafs.pushBack(newLeafNode(parent));
		parent->m_placeables.pushBack(newPlaceableNode(placeable));

		return;
	}

	const Vec4& vMin = volume.getMin();
	const Vec4& vMax = volume.getMax();
	const Vec3 center = (parent->m_aabbMax + parent->m_aabbMin) / 2.0f;

	LeafMask maskX;
	if(vMin.x() > center.x())
	{
		// Only right
		maskX = LeafMask::RIGHT;
	}
	else if(vMax.x() < center.x())
	{
		// Only left
		maskX = LeafMask::LEFT;
	}
	else
	{
		maskX = LeafMask::ALL;
	}

	LeafMask maskY;
	if(vMin.y() > center.y())
	{
		// Only top
		maskY = LeafMask::TOP;
	}
	else if(vMax.y() < center.y())
	{
		// Only bottom
		maskY = LeafMask::BOTTOM;
	}
	else
	{
		maskY = LeafMask::ALL;
	}

	LeafMask maskZ;
	if(vMin.z() > center.z())
	{
		// Only front
		maskZ = LeafMask::FRONT;
	}
	else if(vMax.z() < center.z())
	{
		// Only back
		maskZ = LeafMask::BACK;
	}
	else
	{
		maskZ = LeafMask::ALL;
	}

	const LeafMask maskUnion = maskX & maskY & maskZ;
	ANKI_ASSERT(!!maskUnion && "Should be inside at least one leaf");

	for(U i = 0; i < 8; ++i)
	{
		const LeafMask crntBit = LeafMask(1u << i);

		if(!!(maskUnion & crntBit))
		{
			// Inside the leaf, move deeper

			// Create the leaf
			if(parent->m_children[i] == nullptr)
			{
				Leaf* child = newLeaf();

				// Compute AABB
				Vec3 childAabbMin, childAabbMax;
				computeChildAabb(
					crntBit, parent->m_aabbMin, parent->m_aabbMax, center, child->m_aabbMin, child->m_aabbMax);

				parent->m_children[i] = child;
			}

			// Move deeper
			placeRecursive(volume, placeable, parent->m_children[i], depth + 1);
		}
	}
}

void Octree::computeChildAabb(LeafMask child,
	const Vec3& parentAabbMin,
	const Vec3& parentAabbMax,
	const Vec3& parentAabbCenter,
	Vec3& childAabbMin,
	Vec3& childAabbMax)
{
	ANKI_ASSERT(__builtin_popcount(U32(child)) == 1);

	const Vec3& m = parentAabbMin;
	const Vec3& M = parentAabbMax;
	const Vec3& c = parentAabbCenter;

	if(!!(child & LeafMask::RIGHT))
	{
		// Right
		childAabbMin.x() = c.x();
		childAabbMax.x() = M.x();
	}
	else
	{
		// Left
		childAabbMin.x() = m.x();
		childAabbMax.x() = c.x();
	}

	if(!!(child & LeafMask::TOP))
	{
		// Top
		childAabbMin.y() = c.y();
		childAabbMax.y() = M.y();
	}
	else
	{
		// Bottom
		childAabbMin.y() = m.y();
		childAabbMax.y() = c.y();
	}

	if(!!(child & LeafMask::FRONT))
	{
		// Front
		childAabbMin.z() = c.z();
		childAabbMax.z() = M.z();
	}
	else
	{
		// Back
		childAabbMin.z() = m.z();
		childAabbMax.z() = c.z();
	}
}

void Octree::removeInternal(OctreePlaceable& placeable)
{
	const Bool isPlaced = !placeable.m_leafs.isEmpty();
	if(isPlaced)
	{
		while(!placeable.m_leafs.isEmpty())
		{
			// Pop a leaf node
			LeafNode& leafNode = placeable.m_leafs.getFront();
			placeable.m_leafs.popFront();

			// Iterate the placeables of the leaf
			Bool found = false;
			(void)found;
			for(PlaceableNode& placeableNode : leafNode.m_leaf->m_placeables)
			{
				if(placeableNode.m_placeable == &placeable)
				{
					found = true;
					leafNode.m_leaf->m_placeables.erase(&placeableNode);
					releasePlaceableNode(&placeableNode);
					break;
				}
			}
			ANKI_ASSERT(found);

			// Delete the leaf node
			releaseLeafNode(&leafNode);
		}

		// Cleanup the tree if there are no placeables
		ANKI_ASSERT(m_placeableCount > 0);
		--m_placeableCount;
		if(m_placeableCount == 0)
		{
			cleanupInternal();
			ANKI_ASSERT(m_rootLeaf == nullptr);
		}
	}
}

void Octree::gatherVisibleRecursive(const Frustum& frustum,
	U32 testId,
	OctreeNodeVisibilityTestCallback testCallback,
	void* testCallbackUserData,
	Leaf* leaf,
	DynamicArrayAuto<void*>& out)
{
	ANKI_ASSERT(leaf);

	// Add the placeables that belong to that leaf
	for(PlaceableNode& placeableNode : leaf->m_placeables)
	{
		if(!placeableNode.m_placeable->alreadyVisited(testId))
		{
			ANKI_ASSERT(placeableNode.m_placeable->m_userData);
			out.emplaceBack(placeableNode.m_placeable->m_userData);
		}
	}

	// Move to children leafs
	Aabb aabb;
	for(Leaf* child : leaf->m_children)
	{
		if(child)
		{
			aabb.setMin(child->m_aabbMin);
			aabb.setMax(child->m_aabbMax);

			Bool inside = frustum.insideFrustum(aabb);
			if(inside && testCallback != nullptr)
			{
				inside = testCallback(testCallbackUserData, aabb);
			}

			if(inside)
			{
				gatherVisibleRecursive(frustum, testId, testCallback, testCallbackUserData, child, out);
			}
		}
	}
}

void Octree::cleanupRecursive(Leaf* leaf, Bool& canDeleteLeafUponReturn)
{
	ANKI_ASSERT(leaf);
	canDeleteLeafUponReturn = leaf->m_placeables.getSize() == 0;

	// Do the children
	for(U i = 0; i < 8; ++i)
	{
		Leaf* const child = leaf->m_children[i];
		if(child)
		{
			Bool canDeleteChild;
			cleanupRecursive(child, canDeleteChild);

			if(canDeleteChild)
			{
				releaseLeaf(child);
				leaf->m_children[i] = nullptr;
			}
			else
			{
				canDeleteLeafUponReturn = false;
			}
		}
	}
}

void Octree::cleanupInternal()
{
	if(m_rootLeaf)
	{
		Bool canDeleteLeaf;
		cleanupRecursive(m_rootLeaf, canDeleteLeaf);

		if(canDeleteLeaf)
		{
			releaseLeaf(m_rootLeaf);
			m_rootLeaf = nullptr;
		}
	}
}

void Octree::debugDrawRecursive(const Leaf& leaf, OctreeDebugDrawer& drawer) const
{
	const U32 placeableCount = leaf.m_placeables.getSize();
	const Vec3 color = (placeableCount > 0) ? heatmap(10.0f / placeableCount) : Vec3(0.25f);

	const Aabb box(leaf.m_aabbMin, leaf.m_aabbMax);
	drawer.drawCube(box, Vec4(color, 1.0f));

	for(U i = 0; i < 8; ++i)
	{
		Leaf* const child = leaf.m_children[i];
		if(child)
		{
			debugDrawRecursive(*child, drawer);
		}
	}
}

void Octree::gatherVisibleParallel(const Frustum* frustum,
	U32 testId,
	OctreeNodeVisibilityTestCallback testCallback,
	void* testCallbackUserData,
	DynamicArrayAuto<void*>* out,
	ThreadHive& hive,
	ThreadHiveSemaphore* waitSemaphore,
	ThreadHiveSemaphore*& signalSemaphore)
{
	ANKI_ASSERT(out && frustum);

	// Create the ctx
	GatherParallelCtx* ctx = static_cast<GatherParallelCtx*>(
		hive.allocateScratchMemory(sizeof(GatherParallelCtx), alignof(GatherParallelCtx)));
	ctx->m_octree = this;
	ctx->m_frustum = frustum;
	ctx->m_testId = testId;
	ctx->m_testCallback = testCallback;
	ctx->m_testCallbackUserData = testCallbackUserData;
	ctx->m_out = out;

	// Create the first test ctx
	GatherParallelTaskCtx* taskCtx = static_cast<GatherParallelTaskCtx*>(
		hive.allocateScratchMemory(sizeof(GatherParallelTaskCtx), alignof(GatherParallelTaskCtx)));
	taskCtx->m_ctx = ctx;
	taskCtx->m_leaf = m_rootLeaf;

	// Create signal semaphore
	signalSemaphore = hive.newSemaphore(1);

	// Fire the first task
	ThreadHiveTask task;
	task.m_callback = gatherVisibleTaskCallback;
	task.m_argument = taskCtx;
	task.m_signalSemaphore = signalSemaphore;
	task.m_waitSemaphore = waitSemaphore;

	hive.submitTasks(&task, 1);
}

void Octree::gatherVisibleTaskCallback(void* ud, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* sem)
{
	ANKI_ASSERT(ud);
	GatherParallelTaskCtx* taskCtx = static_cast<GatherParallelTaskCtx*>(ud);
	taskCtx->m_ctx->m_octree->gatherVisibleParallelTask(threadId, hive, sem, *taskCtx);
}

void Octree::gatherVisibleParallelTask(
	U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* sem, GatherParallelTaskCtx& taskCtx)
{
	ANKI_ASSERT(taskCtx.m_ctx && taskCtx.m_leaf);
	GatherParallelCtx& ctx = *taskCtx.m_ctx;

	Leaf* const leaf = taskCtx.m_leaf;
	const Frustum& frustum = *ctx.m_frustum;
	DynamicArrayAuto<void*>& out = *ctx.m_out;
	OctreeNodeVisibilityTestCallback testCallback = ctx.m_testCallback;
	void* testCallbackUserData = ctx.m_testCallbackUserData;
	const U32 testId = ctx.m_testId;

	// Add the placeables that belong to that leaf
	if(leaf->m_placeables.getSize() > 0)
	{
		LockGuard<SpinLock> lock(taskCtx.m_ctx->m_lock);

		for(PlaceableNode& placeableNode : leaf->m_placeables)
		{
			if(!placeableNode.m_placeable->alreadyVisited(testId))
			{
				ANKI_ASSERT(placeableNode.m_placeable->m_userData);
				out.emplaceBack(placeableNode.m_placeable->m_userData);
			}
		}
	}

	// Move to children leafs
	Array<ThreadHiveTask, 8> tasks;
	U taskCount = 0;
	Aabb aabb;
	for(Leaf* child : leaf->m_children)
	{
		if(child)
		{
			aabb.setMin(child->m_aabbMin);
			aabb.setMax(child->m_aabbMax);

			Bool inside = frustum.insideFrustum(aabb);
			if(inside && testCallback != nullptr)
			{
				inside = testCallback(testCallbackUserData, aabb);
			}

			if(inside)
			{
				// New task ctx
				GatherParallelTaskCtx* newTaskCtx = static_cast<GatherParallelTaskCtx*>(
					hive.allocateScratchMemory(sizeof(GatherParallelTaskCtx), alignof(GatherParallelTaskCtx)));
				newTaskCtx->m_ctx = taskCtx.m_ctx;
				newTaskCtx->m_leaf = child;

				// Populate the task
				ThreadHiveTask& task = tasks[taskCount++];
				task.m_callback = gatherVisibleTaskCallback;
				task.m_argument = newTaskCtx;
				task.m_signalSemaphore = sem;
			}
		}
	}

	// Submit all tasks at once
	if(taskCount)
	{
		// At this point do a trick. Increase the semaphore value to keep blocking the tasks that depend on the
		// gather
		sem->increaseSemaphore(taskCount);

		// Submit
		hive.submitTasks(&tasks[0], taskCount);
	}
}

} // end namespace anki
