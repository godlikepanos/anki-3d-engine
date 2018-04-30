// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Octree.h>
#include <anki/collision/Tests.h>
#include <anki/collision/Aabb.h>
#include <anki/collision/Frustum.h>

namespace anki
{

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
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(Vec4(Vec3(m_sceneAabbMin), 0.0f), Vec4(Vec3(m_sceneAabbMax), 0.0f)))
				&& "volume is outside the scene");

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

void Octree::placeRecursive(const Aabb& volume, OctreePlaceable* placeable, Leaf* parent, U32 depth)
{
	ANKI_ASSERT(placeable);
	ANKI_ASSERT(parent);
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(Vec4(parent->m_aabbMin, 0.0f), Vec4(parent->m_aabbMax, 0.0f)))
				&& "Should be inside");

	if(depth == m_maxDepth)
	{
		// Need to stop and bin the placeable to the leaf

		// Checks
		for(const LeafNode& node : placeable->m_leafs)
		{
			ANKI_ASSERT(node.m_leaf != parent && "Already binned. That's wrong");
		}

		for(const PlaceableNode& node : parent->m_placeables)
		{
			ANKI_ASSERT(node.m_placeable != placeable);
		}

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

void Octree::gatherVisibleRecursive(
	const Frustum& frustum, U32 testId, Leaf* leaf, DynamicArrayAuto<OctreePlaceable*>& out)
{
	ANKI_ASSERT(leaf);

	// Add the placeables that belong to that leaf
	for(PlaceableNode& placeableNode : leaf->m_placeables)
	{
		if(!placeableNode.m_placeable->alreadyVisited(testId))
		{
			out.emplaceBack(placeableNode.m_placeable);
		}
	}

	// Move to children leafs
	Aabb aabb;
	for(Leaf* child : leaf->m_children)
	{
		if(child)
		{
			aabb.setMin(Vec4(child->m_aabbMin, 0.0f));
			aabb.setMax(Vec4(child->m_aabbMax, 0.0f));
			if(frustum.insideFrustum(aabb))
			{
				gatherVisibleRecursive(frustum, testId, child, out);
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

} // end namespace anki
