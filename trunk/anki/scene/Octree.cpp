#include "anki/scene/Octree.h"
#include "anki/util/Exception.h"


namespace anki {


//==============================================================================
Octree::Octree(const Aabb& aabb, uint depth)
{
	// Create root
	root = new OctreeNode();
	root->aabb = aabb;

	// Create children
	createSubTree(depth, *root);
}


//==============================================================================
void Octree::createSubTree(uint rdepth, OctreeNode& parent)
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
}


} // end namespace
