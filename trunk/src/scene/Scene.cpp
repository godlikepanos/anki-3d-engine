#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/util/Exception.h"
#include "anki/scene/VisibilityTester.h"
#include "anki/core/ThreadPool.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct UpdateMovablesJob: ThreadJob
{
	Scene::Types<SceneNode>::Iterator movablesBegin;
	U32 movablesCount;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, movablesCount, start, end);

		for(U64 i = start; i < end; i++)
		{
			SceneNode* sn = *(movablesBegin + i);
			Movable* m = sn->getMovable();
			if(m)
			{
				m->update();
			}
		}
	}
};

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
Scene::Scene()
	: alloc(ALLOCATOR_SIZE),
		frameAlloc(ALLOCATOR_SIZE),
		nodes(alloc),
		sectorGroup(this)
{
	ambientCol = Vec3(0.1, 0.05, 0.05) * 2;
}

//==============================================================================
Scene::~Scene()
{}

//==============================================================================
void Scene::registerNode(SceneNode* node)
{
	addC(nodes, node);
	addDict(nameToNode, node);
}

//==============================================================================
void Scene::unregisterNode(SceneNode* node)
{
	removeC(nodes, node);
	removeDict(nameToNode, node);
}

//==============================================================================
void Scene::update(F32 prevUpdateTime, F32 crntTime, Renderer& r)
{
	physics.update(prevUpdateTime, crntTime);

#if 0
	// First do the movable updates
	for(SceneNode* n : nodes)
	{
		Movable* m = n->getMovable();

		if(m)
		{
			m->update();
		}
	}
#else
	ThreadPool& threadPool = ThreadPoolSingleton::get();
	UpdateMovablesJob jobs[ThreadPool::MAX_THREADS];

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].movablesBegin = nodes.begin();
		jobs[i].movablesCount = nodes.size();

		threadPool.assignNewJob(i, &jobs[i]);
	}

	threadPool.waitForAllJobsToFinish();
#endif

	// Then the rest
	for(SceneNode* n : nodes)
	{
		n->frameUpdate(prevUpdateTime, crntTime, Timestamp::getTimestamp());

		Spatial* sp = n->getSpatial();
		if(sp && sp->getSpatialTimestamp() == Timestamp::getTimestamp())
		{
			for(Sector* sector : sectors)
			{
				if(sector->placeSceneNode(n))
				{
					//ANKI_LOGI("Placing: " << n->getName());
					continue;
				}
			}
		}
	}

	doVisibilityTests(*mainCam, r);

#if 0
	for(SceneNode* n : nodes)
	{
		if(n->getSpatial()
			&& n->getSpatial()->getSpatialLastUpdateFrame() == frame)
		{
			std::cout << "Spatial updated on: " << n->getName()
				<< std::endl;
		}

		if(n->getMovable()
			&& n->getMovable()->getMovableLastUpdateFrame() == frame)
		{
			std::cout << "Movable updated on: " << n->getName()
				<< std::endl;
		}

		if(n->getFrustumable()
			&& n->getFrustumable()->getFrustumableLastUpdateFrame() == frame)
		{
			std::cout << "Frustumable updated on: " << n->getName()
				<< std::endl;
		}
	}
#endif
}

//==============================================================================
void Scene::doVisibilityTests(Camera& cam, Renderer& r)
{
	Frustumable* f = cam.getFrustumable();
	ANKI_ASSERT(f != nullptr);
	vtester.test(*f, *this, r);
}

//==============================================================================
SceneNode& Scene::findSceneNode(const char* name)
{
	ANKI_ASSERT(nameToNode.find(name) != nameToNode.end());
	return *(nameToNode.find(name)->second);
}

//==============================================================================
SceneNode* Scene::tryFindSceneNode(const char* name)
{
	Types<SceneNode>::NameToItemMap::iterator it = nameToNode.find(name);
	return (it == nameToNode.end()) ? nullptr : it->second;
}

} // end namespace anki
