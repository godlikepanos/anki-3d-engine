#include "anki/scene/Octree.h"
#include "anki/util/Exception.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"


namespace anki {


//==============================================================================
Octree::Octree(const Aabb& aabb, uchar maxDepth_)
:	maxDepth(maxDepth_)
{
	if(maxDepth < 1)
	{
		maxDepth = 1;
	}

	// Create root
	root = new OctreeNode();
	root->aabb = aabb;

	// Create children
	//createSubTree(maxDepth, *root);
}


//==============================================================================
OctreeNode& Octree::place(const Aabb& aabb)
{
	// Run the recursive
	return place(aabb, 0, *root);
}


//==============================================================================
OctreeNode& Octree::place(const Aabb& aabb, uint depth, OctreeNode& node)
{
	if(depth >= maxDepth)
	{
		return node;
	}

	uint count = 0;
	uint last = 0;
	for(uint i = 0; i < 8; ++i)
	{
		if(CollisionAlgorithmsMatrix::collide(aabb, node.children[i].aabb))
		{
			last = i;
			++count;
		}
	}

	ASSERT(count != 0);

	if(count == 1)
	{
		return place(aabb, depth + 1, node.children[last]);
	}
}


//==============================================================================
/*void Octree::createSubTree(uint rdepth, OctreeNode& parent)
{
	if(rdepth == 0)
	{
		return;
	}

	const Aabb& paabb = parent.getAabb();
	const Vec3& min = paabb.getMin();
	const Vec3& max = paabb.getMax();

	Vec3 d = (max - min) / 2.0;

	// Create children
	for(uint i = 0; i < 2; ++i)
	{
		for(uint j = 0; j < 2; ++j)
		{
			for(uint k = 0; k < 2; ++k)
			{
				Vec3 omin;
				omin.x() = min.x() + d.x() * i;
				omin.y() = min.y() + d.y() * j;
				omin.z() = min.z() + d.z() * k;

				Vec3 omax = omin + d;

				OctreeNode* node = new OctreeNode();
				node->parent = &parent;
				node->aabb = Aabb(omin, omax);

				parent.children[i * 4 + j * 2 + k] = node;
				nodes.push_back(node);

				createSubTree(rdepth - 1, *node);
			} // k
		} // j
	} // i
}*/


} // end namespace
