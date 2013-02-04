#include "anki/scene/Sector.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Light.h"
#include "anki/scene/VisibilityTestResults.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Scene.h"
#include "anki/core/Logger.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct DistanceSortFunctor
{
	Vec3 origin;

	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getSpatial() != nullptr && b->getSpatial() != nullptr);

		F32 dist0 = origin.getDistanceSquared(
			a->getSpatial()->getSpatialOrigin());
		F32 dist1 = origin.getDistanceSquared(
			b->getSpatial()->getSpatialOrigin());

		return dist0 < dist1;
	}
};

//==============================================================================
struct MaterialSortFunctor
{
	Bool operator()(SceneNode* a, SceneNode* b)
	{
		ANKI_ASSERT(a->getRenderable() != nullptr
			&& b->getRenderable() != nullptr);

		return a->getRenderable()->getRenderableMaterial()
			< b->getRenderable()->getRenderableMaterial();
	}
};

//==============================================================================
struct DistanceSortJob: ThreadJob
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;
	Vec3 origin;

	void operator()(U threadId, U threadsCount)
	{
		DistanceSortFunctor comp;
		comp.origin = origin;
		std::sort(nodes, nodes + nodesCount, comp);
	}
};

//==============================================================================
struct MaterialSortJob: ThreadJob
{
	U nodesCount;
	VisibilityTestResults::Container::iterator nodes;

	void operator()(U threadId, U threadsCount)
	{
		std::sort(nodes, nodes + nodesCount, MaterialSortFunctor());
	}
};

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
// SectorGroup                                                                 =
//==============================================================================

//==============================================================================
SectorGroup::SectorGroup(Scene* scene_)
	:	scene(scene_),
		sectors(scene->getAllocator()),
		portals(scene->getAllocator())
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
Sector* SectorGroup::createNewSector(const Aabb& aabb)
{
	Sector* out = ANKI_NEW(Sector, scene->getAllocator(), this, aabb);
	sectors.push_back(out);
	return out;
}

//==============================================================================
Portal* SectorGroup::createNewPortal(Sector* a, Sector* b, 
	const Obb& collisionShape)
{
	ANKI_ASSERT(a && b);
	Portal* out = ANKI_NEW_0(Portal, scene->getAllocator());

	out->sectors[0] = a;
	out->sectors[1] = b;
	out->shape = collisionShape;

	return out;
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
void SectorGroup::doVisibilityTests(SceneNode& sn, VisibilityTest test,
	Renderer* r)
{
	Frustumable* fr = sn.getFrustumable();
	ANKI_ASSERT(fr != nullptr && "sn should be frustumable");

	// Set all sectors to not visible
	for(Sector* sector : sectors)
	{
		sector->visible = false;
	}

	//
	// Find the visible sectors
	//

	SceneVector<Sector*> visibleSectors(scene->getFrameAllocator());

	Spatial* sp = sn.getSpatial();
	ANKI_ASSERT(sp != nullptr);

	// Find the sector that contains the frustumable
	Sector& containerSector = sp->getOctreeNode().getOctree().getSector();
	containerSector.visible = true;
	visibleSectors.push_back(&containerSector);

	// Loop all portals and add other sectors
	for(Portal* portal : portals)
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
				if(r == nullptr || r->doVisibilityTests(portal->shape))
				{
					testAgainstSector->visible = true;
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
	if(r != nullptr)
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
			for(SceneNode* renderable : testResult.renderables)
			{
				if(r->doVisibilityTests(
					renderable->getSpatial()->getOptimalCollisionShape()))
				{
					visible->renderables.push_back(renderable);
				}
			}

			// Then the lights
			for(SceneNode* light : testResult.lights)
			{
				if(r->doVisibilityTests(
					light->getSpatial()->getOptimalCollisionShape()))
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

	ThreadPool& threadPool = ThreadPoolSingleton::get();

	// Sort the renderables in a another thread
	DistanceSortJob dsjob;
	dsjob.nodes = visible->renderables.begin();
	dsjob.nodesCount = visible->renderables.size();
	dsjob.origin = sp->getSpatialOrigin();
	threadPool.assignNewJob(0, &dsjob);

	// The rest of the jobs are dummy
	Array<ThreadJobDummy, ThreadPool::MAX_THREADS>  dummyjobs;
	for(U i = 1; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewJob(i, &dummyjobs[i]);
	}

	// Sort the lights in the main thread
	std::sort(visible->lights.begin(),
		visible->lights.end(), DistanceSortFunctor{sp->getSpatialOrigin()});

	threadPool.waitForAllJobsToFinish();

	//
	// Continue with testing the lights
	//

	for(SceneNode* lsn : visible->lights)
	{
		Light* l = lsn->getLight();
		ANKI_ASSERT(l != nullptr);

		if(l->getShadowEnabled())
		{
			ANKI_ASSERT(lsn->getFrustumable() != nullptr);

			doVisibilityTests(*lsn, 
				(VisibilityTest)(VT_RENDERABLES | VT_ONLY_SHADOW_CASTERS),
				nullptr);
		}
	}
}

} // end namespace anki
