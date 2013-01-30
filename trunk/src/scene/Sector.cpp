#include "anki/scene/Sector.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/VisibilityTestResults.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Scene.h"
#include "anki/core/Logger.h"

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
Sector::Sector(SectorGroup* group_, const Aabb& box)
	: group(group_), octree(this, box, 3),
		portals(group->getScene().getAllocator())
{}

//==============================================================================
Bool Sector::placeSceneNode(SceneNode* sn)
{
	// XXX Optimize

	if(!sn->getSpatial()->getAabb().collide(octree.getRoot().getAabb()))
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
SectorGroup::SectorGroup(Scene* scene_)
	: scene(scene_)
{
	ANKI_ASSERT(scene != nullptr);
}

//==============================================================================
SectorGroup::~SectorGroup()
{
	for(Sector* sector : sectors)
	{
		ANKI_DELETE(sector, scene->getAllocator());
	}

	for(Portal* portal : portals)
	{
		ANKI_DELETE(portal, scene->getAllocator());
	}
}

//==============================================================================
void SectorGroup::placeSceneNode(SceneNode* sn)
{
	ANKI_ASSERT(sn != nullptr);
	Spatial* sp = sn->getSpatial();
	ANKI_ASSERT(sp);
	const Aabb& spAabb = sp->getAabb();

	// Find the candidates first. Sectors overlap, chose the smaller(??!!??)
	Sector* sector = nullptr;
	for(Sector* s : sectors)
	{
		// Spatial inside the sector?
		if(s->getAabb().collide(spAabb))
		{
			// No other candidate?
			if(sector == nullptr)
			{
				sector = s;
			}
			else
			{
				// Other candidata so chose the smaller
				F32 lengthSqA = (sector->getAabb().getMax()
					- sector->getAabb().getMin()).getLengthSquared();

				F32 lengthSqB = (s->getAabb().getMax()
					- s->getAabb().getMin()).getLengthSquared();

				if(lengthSqB < lengthSqA)
				{
					sector = s;
				}
			}
		}
	}

	// Ask the octree to place it
	if(sector != nullptr)
	{
		sector->octree.placeSceneNode(sn);
	}
	else
	{
		ANKI_LOGW("Spatial outside all sectors");
	}
}

//==============================================================================
void SectorGroup::doVisibilityTests(Frustumable& fr, VisibilityTest test)
{
	/// Create the visibility container
	VisibilityTestResults* visible =
		ANKI_NEW(VisibilityTestResults, scene->getFrameAllocator(),
		scene->getFrameAllocator());

	// Find the sectors

	fr.visible = visible;
}

} // end namespace anki
