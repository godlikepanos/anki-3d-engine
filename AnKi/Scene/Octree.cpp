// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Octree.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Collision/Functions.h>
#include <AnKi/Util/ThreadHive.h>

namespace anki {

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

void Octree::place(const Aabb& volume, OctreePlaceable* placeable, Bool updateActualSceneBounds)
{
	ANKI_ASSERT(placeable);
	ANKI_ASSERT(testCollision(volume, Aabb(m_sceneAabbMin, m_sceneAabbMax)) && "volume is outside the scene");

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

	// Update the actual scene bounds
	if(updateActualSceneBounds)
	{
		m_actualSceneAabbMin = m_actualSceneAabbMin.min(volume.getMin().xyz());
		m_actualSceneAabbMax = m_actualSceneAabbMax.max(volume.getMax().xyz());
	}
}

void Octree::placeAlwaysVisible(OctreePlaceable* placeable)
{
	ANKI_ASSERT(placeable);

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

	// Connect placeable and leaf
	placeable->m_leafs.pushBack(newLeafNode(m_rootLeaf));
	m_rootLeaf->m_placeables.pushBack(newPlaceableNode(placeable));

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
	ANKI_ASSERT(testCollision(volume, Aabb(parent->m_aabbMin, parent->m_aabbMax)) && "Should be inside");

	if(depth == m_maxDepth || volumeTotallyInsideLeaf(volume, *parent))
	{
		// Need to stop and bin the placeable to the leaf

		// Checks
#if ANKI_ENABLE_ASSERTIONS
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
		maskX = LeafMask::kRight;
	}
	else if(vMax.x() < center.x())
	{
		// Only left
		maskX = LeafMask::kLeft;
	}
	else
	{
		maskX = LeafMask::kAll;
	}

	LeafMask maskY;
	if(vMin.y() > center.y())
	{
		// Only top
		maskY = LeafMask::kTop;
	}
	else if(vMax.y() < center.y())
	{
		// Only bottom
		maskY = LeafMask::kBottom;
	}
	else
	{
		maskY = LeafMask::kAll;
	}

	LeafMask maskZ;
	if(vMin.z() > center.z())
	{
		// Only front
		maskZ = LeafMask::kFront;
	}
	else if(vMax.z() < center.z())
	{
		// Only back
		maskZ = LeafMask::kBack;
	}
	else
	{
		maskZ = LeafMask::kAll;
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
				computeChildAabb(crntBit, parent->m_aabbMin, parent->m_aabbMax, center, child->m_aabbMin,
								 child->m_aabbMax);

				parent->m_children[i] = child;
			}

			// Move deeper
			placeRecursive(volume, placeable, parent->m_children[i], depth + 1);
		}
	}
}

void Octree::computeChildAabb(LeafMask child, const Vec3& parentAabbMin, const Vec3& parentAabbMax,
							  const Vec3& parentAabbCenter, Vec3& childAabbMin, Vec3& childAabbMax)
{
	ANKI_ASSERT(__builtin_popcount(U32(child)) == 1);

	const Vec3& m = parentAabbMin;
	const Vec3& M = parentAabbMax;
	const Vec3& c = parentAabbCenter;

	if(!!(child & LeafMask::kRight))
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

	if(!!(child & LeafMask::kTop))
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

	if(!!(child & LeafMask::kFront))
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
			LeafNode* leafNode = placeable.m_leafs.popFront();

			// Iterate the placeables of the leaf
			[[maybe_unused]] Bool found = false;
			for(PlaceableNode& placeableNode : leafNode->m_leaf->m_placeables)
			{
				if(placeableNode.m_placeable == &placeable)
				{
					found = true;
					leafNode->m_leaf->m_placeables.erase(&placeableNode);
					releasePlaceableNode(&placeableNode);
					break;
				}
			}
			ANKI_ASSERT(found);

			// Delete the leaf node
			releaseLeafNode(leafNode);
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
	const U32 placeableCount = U32(leaf.m_placeables.getSize());
	const Vec3 color = (placeableCount > 0) ? heatmap(10.0f / F32(placeableCount)) : Vec3(0.25f);

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

} // end namespace anki
