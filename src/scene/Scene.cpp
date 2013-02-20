#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/util/Exception.h"
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
static void updateSceneNode(SceneNode& sn, F32 prevUpdateTime,
	F32 crntTime, SectorGroup& sectorGroup)
{
	sn.frameUpdate(prevUpdateTime, crntTime, Timestamp::getTimestamp());

	// Do some spatial stuff
	Spatial* sp = sn.getSpatial();
	if(sp)
	{
		if(sp->getSpatialTimestamp() == Timestamp::getTimestamp())
		{
			sectorGroup.placeSceneNode(&sn);
		}
		sp->disableFlags(Spatial::SF_VISIBLE_ANY);
	}

	// Do some frustumable stuff
	Frustumable* fr = sn.getFrustumable();
	if(fr)
	{
		fr->setVisibilityTestResults(nullptr);
	}

	// Do some renderable stuff
	Renderable* r = sn.getRenderable();
	if(r)
	{
		r->resetFrame();
	}
}

//==============================================================================
struct UpdateSceneNodesJob: ThreadJob
{
	Scene::Types<SceneNode>::Iterator sceneNodesBegin;
	U32 sceneNodesCount;
	F32 prevUpdateTime;
	F32 crntTime;
	SectorGroup* sectorGroup;

	void operator()(U threadId, U threadsCount)
	{
		U64 start, end;
		choseStartEnd(threadId, threadsCount, sceneNodesCount, start, end);

		for(U64 i = start; i < end; i++)
		{
			SceneNode* n = *(sceneNodesBegin + i);
			updateSceneNode(*n, prevUpdateTime, crntTime, *sectorGroup);
		}
	}
};

//==============================================================================
// Scene                                                                       =
//==============================================================================

//==============================================================================
Scene::Scene()
	:	alloc(ANKI_CFG_SCENE_ALLOCATOR_SIZE),
		frameAlloc(ANKI_CFG_SCENE_FRAME_ALLOCATOR_SIZE),
		nodes(alloc),
		sectorGroup(this)
{
	nodes.reserve(ANKI_CFG_SCENE_NODES_AVERAGE_COUNT);

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
void Scene::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	frameAlloc.reset();

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
#if 0
	for(SceneNode* n : nodes)
	{
		updateSceneNode(*n, prevUpdateTime, crntTime, *sectorGroup);
	}
#else
	UpdateSceneNodesJob jobs2[ThreadPool::MAX_THREADS];

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesJob& job = jobs2[i];

		job.sceneNodesBegin = nodes.begin();
		job.sceneNodesCount = nodes.size();
		job.prevUpdateTime = prevUpdateTime;
		job.crntTime = crntTime;
		job.sectorGroup = &sectorGroup;

		threadPool.assignNewJob(i, &job);
	}
#endif

	doVisibilityTests(*mainCam, *this, renderer);

	/*sectorGroup.doVisibilityTests(*mainCam,
		VisibilityTest(VT_RENDERABLES | VT_LIGHTS), &r);*/

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
