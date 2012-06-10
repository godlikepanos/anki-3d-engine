#include "anki/scene/Octree.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"


namespace anki {


//==============================================================================
Octree::Octree(const Aabb& aabb, uint8_t maxDepth_, float looseness_)
:	maxDepth(maxDepth_ < 1 ? 1 : maxDepth_),
 	looseness(looseness_)
{
	// Create root
	root = new OctreeNode(aabb, NULL);
	nodes.push_back(root);
}


//==============================================================================
OctreeNode* Octree::place(const Aabb& aabb)
{
	if(CollisionAlgorithmsMatrix::collide(aabb, root->getAabb()))
	{
		// Run the recursive
		return place(aabb, 0, *root);
	}
	else
	{
		return NULL;
	}
}


//==============================================================================
OctreeNode* Octree::place(const Aabb& aabb, uint depth, OctreeNode& node)
{
	if(depth >= maxDepth)
	{
		return &node;
	}


	for(uint i = 0; i < 2; ++i)
	{
		for(uint j = 0; j < 2; ++j)
		{
			for(uint k = 0; k < 2; ++k)
			{
				uint id = i * 4 + j * 2 + k;

				Aabb childAabb;
				if(node.getChildren()[id] != NULL)
				{
					childAabb = node.getChildren()[id]->getAabb();
				}
				else
				{
					calcAabb(i, j, k, node.getAabb(), childAabb);
				}


				// If aabb its completely inside the target
				if(aabb.getMax() <= childAabb.getMax() &&
					aabb.getMin() >= childAabb.getMin())
				{
					// Create new node if needed
					if(node.getChildren()[id] == NULL)
					{
						OctreeNode* newNode = new OctreeNode(childAabb, &node);
						node.addChild(id, *newNode);
						nodes.push_back(newNode);
					}

					return place(aabb, depth + 1, *node.getChildren()[id]);
				}
			} // k
		} // j
	} // i

	return &node;
}


//==============================================================================
void Octree::calcAabb(uint i, uint j, uint k, const Aabb& paabb,
	Aabb& out) const
{
	const Vec3& min = paabb.getMin();
	const Vec3& max = paabb.getMax();
	Vec3 d = (max - min) / 2.0;

	Vec3 omin;
	omin.x() = min.x() + d.x() * i;
	omin.y() = min.y() + d.y() * j;
	omin.z() = min.z() + d.z() * k;

	Vec3 omax = omin + d;

	// Scale the AABB with looseness
	float tmp0 = (1.0 + looseness) / 2.0;
	float tmp1 = (1.0 - looseness) / 2.0;
	Vec3 nomin = tmp0 * omin + tmp1 * omax;
	Vec3 nomax = tmp0 * omax + tmp1 * omin;

	// Crop to fit the parent's AABB
	for(uint n = 0; n < 3; ++n)
	{
		if(nomin[n] < min[n])
		{
			nomin[n] = min[n];
		}

		if(nomax[n] > max[n])
		{
			nomax[n] = max[n];
		}
	}

	out = Aabb(nomin, nomax);
}


} // end namespace
