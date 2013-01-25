#include "anki/scene/Sector.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/SceneNode.h"
#include "anki/collision/CollisionAlgorithmsMatrix.h"

namespace anki {

//==============================================================================
// Portal                                                                      =
//==============================================================================

//==============================================================================
Portal::Portal()
{
	sectors[0] = sectors[1] = nullptr;
}

//==============================================================================
// Sector                                                                      =
//==============================================================================

//==============================================================================
Sector::Sector(const SceneAllocator<U8>& alloc_, const Aabb& box)
	: octree(alloc_, box, 3), portals(alloc_)
{}

//==============================================================================
Bool Sector::placeSceneNode(SceneNode* sn)
{
	// XXX Optimize

	if(!CollisionAlgorithmsMatrix::collide(sn->getSpatial()->getAabb(),
		octree.getRoot().getAabb()))
	{
		return false;
	}

	octree.placeSceneNode(sn);
	return true;
}

//==============================================================================
// Sector                                                                      =
//==============================================================================

//==============================================================================
SectorGroup::SectorGroup(const SceneAllocator<U8>& alloc_)
	: alloc(alloc_)
{}

//==============================================================================
SectorGroup::~SectorGroup()
{
	for(Sector* sector : sectors)
	{
		ANKI_DELETE(sector, alloc);
	}

	for(Portal* portal : portals)
	{
		ANKI_DELETE(portal, alloc);
	}
}

//==============================================================================
Bool SectorGroup::placeSceneNode(SceneNode* sp)
{
	// XXX
	return false;
}

} // end namespace anki
