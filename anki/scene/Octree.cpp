#include "anki/scene/Octree.h"
#include "anki/util/Exception.h"


namespace anki {


//==============================================================================
Octree::Octree(const Aabb& aabb, uint depth)
{
	// Create the nodes
}


//==============================================================================
void Octree::createSubTree(const Aabb& paabb, uint rdepth, OctreeNode* parent)
{
	if(rdepth == 0)
	{
		return;
	}

	// Create children AABBs
	boost::array<Aabb, 8> aabbs;
	Aabb common = Aabb(paabb.get);



	for(uint i = 0; i < 8; ++i)
	{

	}
}


} // end namespace
