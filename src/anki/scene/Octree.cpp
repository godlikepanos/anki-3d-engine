// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Octree.h>
#include <anki/collision/Tests.h>
#include <anki/collision/Aabb.h>

namespace anki
{

void Octree::init(const Vec3& sceneMin, const Vec3& sceneMax, U32 maxDepth)
{
	ANKI_ASSERT(sceneMin < sceneMax);
	ANKI_ASSERT(maxDepth > 0);

	m_maxDepth = maxDepth;

	OctreeLeaf& root = newLeaf();
	root.m_aabbMin = sceneMin;
	root.m_aabbMax = sceneMax;
	m_rootLeaf = &root;
}

void Octree::placeElement(const Aabb& volume, OctreeElement& element)
{
	ANKI_ASSERT(m_rootLeaf);
	placeElementRecursive(volume, element, *m_rootLeaf);
}

void Octree::placeElementRecursive(const Aabb& volume, OctreeElement& element, OctreeLeaf& parent)
{
	ANKI_ASSERT(testCollisionShapes(volume, Aabb(Vec4(parent.m_aabbMin, 0.0f), Vec4(parent.m_aabbMax, 0.0f)))
				&& "Should be inside");

	const Vec3 center = (parent.m_aabbMax + parent.m_aabbMin) / 2.0f;
}

OctreeLeaf& Octree::newLeaf()
{
	OctreeLeaf* out = nullptr;

	// Try find one in the storages
	for(LeafStorage& storage : m_leafStorages)
	{
		if(storage.m_unusedLeafCount > 0)
		{
			// Pop an element
			--storage.m_unusedLeafCount;
			out = &storage.m_leafs[storage.m_unusedLeafCount];
			break;
		}
	}

	if(out == nullptr)
	{
		// Need to create a new storage

		m_leafStorages.resize(m_alloc, m_leafStorages.getSize() + 1);
		LeafStorage& storage = m_leafStorages.getBack();

		storage.m_leafs = {m_alloc.newArray<OctreeLeaf>(LEAFES_PER_STORAGE), LEAFES_PER_STORAGE};
		storage.m_unusedLeafCount = LEAFES_PER_STORAGE;

		storage.m_unusedLeafsStack = {m_alloc.newArray<U16>(LEAFES_PER_STORAGE), LEAFES_PER_STORAGE};
		for(U i = 0; i < LEAFES_PER_STORAGE; ++i)
		{
			storage.m_unusedLeafsStack[i] = LEAFES_PER_STORAGE - (i + 1);
		}

		// Allocate one
		out = &storage.m_leafs[0];
		--storage.m_unusedLeafCount;
	}

	// Init the leaf
	zeroMemory(*out);

	ANKI_ASSERT(out);
	return *out;
}

void Octree::releaseLeaf(OctreeLeaf* leaf)
{
	ANKI_ASSERT(leaf);

	for(LeafStorage& storage : m_leafStorages)
	{
		if(leaf >= &storage.m_leafs.getFront() && leaf <= &storage.m_leafs.getBack())
		{
			// Found its storage

			const U idx = leaf - storage.m_leafs.getBegin();
			storage.m_unusedLeafsStack[storage.m_unusedLeafCount] = idx;
			++storage.m_unusedLeafCount;
			return;
		}
	}

	ANKI_ASSERT(!"Not found");
}

} // end namespace anki