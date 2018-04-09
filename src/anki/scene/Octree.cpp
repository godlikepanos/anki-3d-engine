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

	OctreeLeaf& root = newLeaf();
	m_rootLeaf = &root;
}

void Octree::placeElement(const Aabb& volume, OctreeElement* element)
{
	ANKI_ASSERT(m_rootLeaf);
	ANKI_ASSERT(element);
	placeElementRecursive(volume, element, *m_rootLeaf, m_sceneAabbMin, m_sceneAabbMax);
}

void Octree::placeElementRecursive(
	const Aabb& volume, OctreeElement* element, OctreeLeaf& parent, const Vec3& aabbMin, const Vec3& aabbMax)
{
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(Vec4(aabbMin, 0.0f), Vec4(aabbMax, 0.0f))) && "Should be inside");

	const Vec4& vMin = volume.getMin();
	const Vec4& vMax = volume.getMax();
	const Vec3 center = (aabbMax + aabbMin) / 2.0f;

	LeafMask maskX;
	if(vMin.x() > center.x())
	{
		// Only right
		maskX = LeafMask::PX_PY_PZ | LeafMask::PX_PY_NZ | LeafMask::PX_NY_PZ | LeafMask::PX_NY_NZ;
	}
	else if(vMax.x() < center.x())
	{
		// Only left
		maskX = LeafMask::NX_PY_PZ | LeafMask::NX_PY_NZ | LeafMask::NX_NY_PZ | LeafMask::NX_NY_NZ;
	}
	else
	{
		maskX = LeafMask::ALL;
	}

	LeafMask maskY;
	if(vMin.y() > center.y())
	{
		// Only top
		maskY = LeafMask::PX_PY_PZ | LeafMask::PX_PY_NZ | LeafMask::NX_PY_PZ | LeafMask::NX_PY_NZ;
	}
	else if(vMax.y() < center.y())
	{
		// Only bottom
		maskY = LeafMask::PX_NY_PZ | LeafMask::PX_NY_NZ | LeafMask::NX_NY_PZ | LeafMask::NX_NY_NZ;
	}
	else
	{
		maskY = LeafMask::ALL;
	}

	LeafMask maskZ;
	if(vMin.z() > center.z())
	{
		// Only front
		maskZ = LeafMask::PX_PY_PZ | LeafMask::PX_NY_PZ | LeafMask::NX_PY_PZ | LeafMask::NX_NY_PZ;
	}
	else if(vMax.z() < center.z())
	{
		// Only back
		maskZ = LeafMask::PX_PY_NZ | LeafMask::PX_NY_NZ | LeafMask::NX_PY_NZ | LeafMask::NX_NY_NZ;
	}
	else
	{
		maskZ = LeafMask::ALL;
	}

	LeafMask maskUnion = maskX & maskY & maskZ;
	ANKI_ASSERT(!!maskUnion);

	// TODO
}

} // end namespace anki
