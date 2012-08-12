#include "anki/scene/Octree.h"
#include "anki/scene/Spatial.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"

namespace anki {

//==============================================================================
Octree::Octree(const Aabb& aabb, uint8_t maxDepth_, float looseness_)
	: maxDepth(maxDepth_ < 1 ? 1 : maxDepth_), looseness(looseness_),
		root(aabb, nullptr)
{}

//==============================================================================
void Octree::placeSpatial(Spatial* sp)
{
	OctreeNode* toBePlacedNode = place(sp->getAabb());

	if(toBePlacedNode == nullptr)
	{
		ANKI_LOGI("Outside the whole tree");
		// Outside the whole tree
		return;
	}

	OctreeNode* crntNode = sp->getOctreeNode();

	if(crntNode == toBePlacedNode)
	{
		ANKI_LOGI("Don't place in the same");
		// Don't place in the same
		return;
	}

	// Remove from current node
	if(crntNode)
	{
		Vector<Spatial*>::iterator it =
			std::find(crntNode->spatials.begin(), crntNode->spatials.end(), sp);

		ANKI_ASSERT(it != crntNode->spatials.end());
		crntNode->spatials.erase(it);
	}

	// Add to new one node
	toBePlacedNode->spatials.push_back(sp);
	sp->setOctreeNode(toBePlacedNode);
}

//==============================================================================
OctreeNode* Octree::place(const Aabb& aabb)
{
	if(CollisionAlgorithmsMatrix::collide(aabb, root.aabb))
	{
		// Run the recursive method
		return place(aabb, 0, root);
	}
	else
	{
		// Its completely outside the octree
		return nullptr;
	}
}

//==============================================================================
OctreeNode* Octree::place(const Aabb& aabb, uint32_t depth, OctreeNode& node)
{
	if(depth >= maxDepth)
	{
		return &node;
	}

	for(uint32_t i = 0; i < 2; ++i)
	{
		for(uint32_t j = 0; j < 2; ++j)
		{
			for(uint32_t k = 0; k < 2; ++k)
			{
				uint32_t id = i * 4 + j * 2 + k;

				// Get the node's AABB. If there is no node then calculate the
				// AABB
				Aabb childAabb;
				if(node.children[id].get() != nullptr)
				{
					// There is a child
					childAabb = node.children[id]->aabb;
				}
				else
				{
					// Calculate
					calcAabb(i, j, k, node.aabb, childAabb);
				}

				// If aabb its completely inside the target
				if(aabb.getMax() <= childAabb.getMax() &&
					aabb.getMin() >= childAabb.getMin())
				{
					// Go deeper
					if(node.children[id].get() == nullptr)
					{
						// Create new node if needed
						OctreeNode* newNode = new OctreeNode(childAabb, &node);
						node.addChild(id, newNode);
					}

					return place(aabb, depth + 1, *node.children[id]);
				}
			} // k
		} // j
	} // i

	return &node;
}

//==============================================================================
void Octree::calcAabb(uint32_t i, uint32_t j, uint32_t k, const Aabb& paabb,
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
	for(uint32_t n = 0; n < 3; ++n)
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

} // end namespace anki
