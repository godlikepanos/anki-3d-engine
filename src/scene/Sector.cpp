// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Sector.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Light.h"
#include "anki/scene/Visibility.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
// Portal                                                                      =
//==============================================================================

//==============================================================================
Portal::Portal()
{
	sectors[0] = sectors[1] = nullptr;
	open = true;
}

//==============================================================================
// Sector                                                                      =
//==============================================================================

//==============================================================================
Sector::Sector(SectorGroup* group_, const Aabb& box)
	:	group(group_),
		portals(group->getSceneGraph().getAllocator())
{
	// Reserve some space for portals
	portals.reserve(AVERAGE_PORTALS_PER_SECTOR);
}

//==============================================================================
Bool Sector::placeSceneNode(SceneNode* sn)
{
#if 0
	// XXX Optimize

	SpatialComponent* sp = sn->getSpatialComponent();
	ANKI_ASSERT(sp);
	if(!sp->getAabb().collide(octree.getRoot().getAabb()))
	{
		return false;
	}

	octree.placeSceneNode(sn);
#endif
	return true;
}

//==============================================================================
void Sector::addNewPortal(Portal* portal)
{
	ANKI_ASSERT(portal);
	ANKI_ASSERT(
		std::find(portals.begin(), portals.end(), portal) == portals.end()
		&& "Portal found in the container");

	portals.push_back(portal);
}

//==============================================================================
void Sector::removePortal(Portal* portal)
{
	ANKI_ASSERT(portal);
	SceneVector<Portal*>::iterator it =
		std::find(portals.begin(), portals.end(), portal);

	ANKI_ASSERT(it != portals.end());
	portals.erase(it);
}

//==============================================================================
// SectorGroup                                                                 =
//==============================================================================

//==============================================================================
SectorGroup::SectorGroup(SceneGraph* scene_)
	:	scene(scene_),
		sectors(scene->getAllocator()),
		portals(scene->getAllocator())
{
	ANKI_ASSERT(scene != nullptr);
}

//==============================================================================
SectorGroup::~SectorGroup()
{
#if 0
	for(Sector* sector : sectors)
	{
		ANKI_DELETE(sector, scene->getAllocator());
	}

	for(Portal* portal : portals)
	{
		ANKI_DELETE(portal, scene->getAllocator());
	}
#endif
}

//==============================================================================
Sector* SectorGroup::createNewSector(const Aabb& aabb)
{
#if 0
	Sector* out = ANKI_NEW(Sector, scene->getAllocator(), this, aabb);
	sectors.push_back(out);
	return out;
#endif
	return nullptr;
}

//==============================================================================
Portal* SectorGroup::createNewPortal(Sector* a, Sector* b, 
	const Obb& collisionShape)
{
#if 0
	ANKI_ASSERT(a && b);
	Portal* out = ANKI_NEW_0(Portal, scene->getAllocator());

	out->sectors[0] = a;
	out->sectors[1] = b;
	out->shape = collisionShape;

	portals.push_back(out);

	a->addNewPortal(out);
	b->addNewPortal(out);

	return out;
#endif
	return nullptr;
}

//==============================================================================
void SectorGroup::placeSceneNode(SceneNode* sn)
{
#if 0
	ANKI_ASSERT(sn != nullptr);
	SpatialComponent* sp = sn->getSpatial();
	ANKI_ASSERT(sp);
	const Vec3& spPos = sp->getSpatialOrigin();

	// Find the candidates first. Sectors overlap, chose the smaller(??!!??)
	Sector* sector = nullptr;
	for(Sector* s : sectors)
	{
		// Find if the spatia's position is inside the sector
		Bool inside = true;
		for(U i = 0; i < 3; i++)
		{
			if(spPos[i] > s->getAabb().getMax()[i]
				|| spPos[i] < s->getAabb().getMin()[i])
			{
				inside = false;
				continue;
			}
		}

		if(!inside)
		{
			// continue
		}
		else
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
		} // end if inside
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
#endif
}

//==============================================================================
void SectorGroup::doVisibilityTests(SceneNode& sn, VisibilityTest test,
	Renderer* r)
{
#if 0
	// Set all sectors to not visible
	for(Sector* sector : sectors)
	{
		sector->visibleBy = VB_NONE;
	}

	doVisibilityTestsInternal(sn, test, r, VB_CAMERA);
#endif
}

//==============================================================================
void SectorGroup::doVisibilityTestsInternal(SceneNode& sn, VisibilityTest test,
	Renderer* r, VisibleBy visibleBy)
{
#if 0
	Frustumable* fr = sn.getFrustumable();
	ANKI_ASSERT(fr != nullptr && "sn should be frustumable");
	fr->visible = nullptr;

	SpatialComponent* sp = sn.getSpatial();
	ANKI_ASSERT(sp != nullptr && "sn should be spatial as well");

	// sn is not placed in any octree
	if(sp->getOctreeNode() == nullptr)
	{
		return;
	}

	//
	// Find the visible sectors
	//

	SceneFrameVector<Sector*> visibleSectors(scene->getFrameAllocator());

	// Find the sector that contains the frustumable
	Sector& containerSector = sp->getOctreeNode()->getOctree().getSector();
	containerSector.visibleBy |= visibleBy;
	visibleSectors.push_back(&containerSector);

	// Loop all sector portals and add other sectors
	for(Portal* portal : containerSector.portals)
	{
		if(ANKI_UNLIKELY(!portal->open))
		{
			continue;
		}

		// Get the "other" sector of that portal
		Sector* testAgainstSector;

		if(portal->sectors[0] == &containerSector)
		{
			testAgainstSector = portal->sectors[1];
		}
		else
		{
			ANKI_ASSERT(portal->sectors[1] == &containerSector);
			testAgainstSector = portal->sectors[0];
		}
		ANKI_ASSERT(testAgainstSector != nullptr);

		// Search if portal is in the container from another portal
		SceneVector<Sector*>::iterator it = std::find(visibleSectors.begin(),
			visibleSectors.end(), testAgainstSector);

		if(it == visibleSectors.end())
		{
			// Not found so test the portal

			// Portal is visible
			if(fr->insideFrustum(portal->shape))
			{
				/*if(r == nullptr || r->doVisibilityTests(portal->shape))*/
				{
					testAgainstSector->visibleBy |= visibleBy;
					visibleSectors.push_back(testAgainstSector);
				}
			}
		}
	}

	//
	// For all visible sectors do the tests
	//

	// Create a few VisibilityTestResults to pass to every octree
	SceneVector<VisibilityTestResults> testResults(scene->getFrameAllocator());
	testResults.resize(visibleSectors.size(), 
		VisibilityTestResults(scene->getFrameAllocator()));
	U renderablesCount = 0;
	U lightsCount = 0;

	// Run tests for every octree
	for(U i = 0; i < visibleSectors.size(); i++)
	{
		Sector* sector = visibleSectors[i];

		sector->octree.doVisibilityTests(sn, test, testResults[i]);

		renderablesCount += testResults[i].renderables.size();
		lightsCount += testResults[i].lights.size();
	}

	// If you don't want lights then the octree should do tests with them
	ANKI_ASSERT(!((test & VT_LIGHTS) == 0 && lightsCount != 0));
	// Same as ^
	ANKI_ASSERT(!((test & VT_RENDERABLES) == 0 && renderablesCount != 0));

	//
	// Combine test results and try doing renderer tests
	//

	// Create the visibility container
	VisibilityTestResults* visible =
		ANKI_NEW(VisibilityTestResults, scene->getFrameAllocator(),
		scene->getFrameAllocator());

	// Set the sizes to save some moves
	visible->renderables.reserve(renderablesCount);
	visible->lights.reserve(lightsCount);

	// Iterate previous test results and append to the combined one
	if(r == nullptr)
	{
		for(VisibilityTestResults& testResult : testResults)
		{
			visible->renderables.insert(
				visible->renderables.end(), 
				testResult.renderables.begin(), 
				testResult.renderables.end());

			visible->lights.insert(
				visible->lights.end(), 
				testResult.lights.begin(), 
				testResult.lights.end());
		}
	}
	else
	{
		for(VisibilityTestResults& testResult : testResults)
		{
			// First the renderables
			for(VisibleNode& renderable : testResult.renderables)
			{
				/*XXX if(r->getTiler().test2(
					renderable->getSpatial()->getOptimalCollisionShape()))*/
				{
					visible->renderables.push_back(renderable);
				}
			}

			// Then the lights
			for(VisibleNode& light : testResult.lights)
			{
				/*XXX if(r->doVisibilityTests(
					light->getSpatial()->getOptimalCollisionShape()))*/
				{
					visible->lights.push_back(light);
				}
			}
		}
	}

	// The given frustumable is finished
	fr->visible = visible;

	//
	// Sort
	//

	Threadpool.h& threadPool = Threadpool.hSingleton::get();

	// Sort the renderables in a another thread
	DistanceSortJob dsjob;
	dsjob.nodes = visible->renderables.begin();
	dsjob.nodesCount = visible->renderables.size();
	dsjob.origin = sp->getSpatialOrigin();
	threadPool.assignNewJob(0, &dsjob);

	// The rest of the jobs are dummy
	Array<ThreadJobDummy, Threadpool.h::MAX_THREADS>  dummyjobs;
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewJob(i, &dummyjobs[i]);
	}

	// Sort the lights in the main thread
	std::sort(visible->lights.begin(),
		visible->lights.end(), DistanceSortFunctor{sp->getSpatialOrigin()});

	threadPool.waitForAllJobsToFinish();

	//
	// Continue by testing the lights
	//

	for(VisibleNode& lsn : visible->lights)
	{
		Light* l = lsn.node->getLight();
		ANKI_ASSERT(l != nullptr);

		if(l->getShadowEnabled())
		{
			ANKI_ASSERT(lsn.node->getFrustumable() != nullptr);

			doVisibilityTestsInternal(*lsn.node,
				(VisibilityTest)(VT_RENDERABLES | VT_ONLY_SHADOW_CASTERS),
				nullptr, VB_LIGHT);
		}
	}
#endif
}

} // end namespace anki
