// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Octree.h>
#include <anki/collision/Tests.h>
#include <anki/collision/Aabb.h>

namespace anki
{

void Octree::init(const Vec3& sceneAabbMin, const Vec3& sceneAabbMax, U32 maxDepth)
{
	ANKI_ASSERT(sceneAabbMin < sceneAabbMax);
	ANKI_ASSERT(maxDepth > 0);

	m_maxDepth = maxDepth;
	m_sceneAabbMin = sceneAabbMin;
	m_sceneAabbMax = sceneAabbMax;

	m_rootLeaf = newLeaf();
}

void Octree::place(const Aabb& volume, OctreeHandle* handle)
{
	ANKI_ASSERT(m_rootLeaf);
	ANKI_ASSERT(handle);

	// Remove the handle from the Octree...
	// TODO

	// .. and re-place it
	placeRecursive(volume, handle, m_rootLeaf, m_sceneAabbMin, m_sceneAabbMax, 0);
}

void Octree::placeRecursive(
	const Aabb& volume, OctreeHandle* handle, Leaf* parent, const Vec3& aabbMin, const Vec3& aabbMax, U32 depth)
{
	ANKI_ASSERT(handle);
	ANKI_ASSERT(parent);
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(Vec4(aabbMin, 0.0f), Vec4(aabbMax, 0.0f))) && "Should be inside");

	if(depth == m_maxDepth)
	{
		// Need to stop and bin the handle to the leaf

		// Checks
		for(const LeafNode& node : handle->m_leafs)
		{
			ANKI_ASSERT(node.m_leaf != parent && "Already binned. That's wrong");
		}

		// Connect handle and leaf
		handle->m_leafs.pushBack(newLeafNode(parent));
		parent->m_handles.pushBack(newHandleNode(handle));

		return;
	}

	const Vec4& vMin = volume.getMin();
	const Vec4& vMax = volume.getMax();
	const Vec3 center = (aabbMax + aabbMin) / 2.0f;

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
			if(parent->m_leafs[i] == nullptr)
			{
				parent->m_leafs[i] = newLeaf();
			}

			// Compute AABB
			Vec3 childAabbMin, childAabbMax;
			computeChildAabb(crntBit, aabbMin, aabbMax, center, childAabbMin, childAabbMax);

			// Move deeper
			placeRecursive(volume, handle, parent->m_leafs[i], childAabbMin, childAabbMax, depth + 1);
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

} // end namespace anki
