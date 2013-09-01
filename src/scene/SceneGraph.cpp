#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/util/Exception.h"
#include "anki/core/ThreadPool.h"
#include "anki/core/Counters.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct UpdateMovablesJob: ThreadJob
{
	SceneGraph::Types<SceneNode>::Iterator movablesBegin;
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
	sn.frameUpdate(prevUpdateTime, crntTime, getGlobTimestamp());

	// Do some spatial stuff
	Spatial* sp = sn.getSpatial();
	if(sp)
	{
		if(sp->getSpatialTimestamp() >= getGlobTimestamp())
		{
			sp->update();
			//sectorGroup.placeSceneNode(&sn);
		}
		sp->resetFrame();
	}

	// Do some frustumable stuff
	Frustumable* fr = sn.getFrustumable();
	if(fr)
	{
		fr->resetFrame();
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
	SceneGraph::Types<SceneNode>::Iterator sceneNodesBegin;
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
SceneGraph::SceneGraph()
	:	alloc(ANKI_SCENE_ALLOCATOR_SIZE),
		frameAlloc(ANKI_SCENE_FRAME_ALLOCATOR_SIZE),
		nodes(alloc),
		sectorGroup(this),
		events(this)
{
	nodes.reserve(ANKI_SCENE_OPTIMAL_SCENE_NODES_COUNT);

	ambientCol = Vec3(0.0);
}

//==============================================================================
SceneGraph::~SceneGraph()
{}

//==============================================================================
void SceneGraph::registerNode(SceneNode* node)
{
	addC(nodes, node);

	// Add to dict if it has name
	if(node->getName())
	{
		addDict(nameToNode, node);
	}
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	removeC(nodes, node);

	// Remove from dict if it has name
	if(node->getName())
	{
		removeDict(nameToNode, node);
	}
}

//==============================================================================
void SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	ANKI_ASSERT(mainCam);

	ANKI_COUNTER_START_TIMER(C_SCENE_UPDATE_TIME);

	ThreadPool& threadPool = ThreadPoolSingleton::get();
	(void)threadPool;
	frameAlloc.reset();

	// XXX Do that in parallel
	physics.update(prevUpdateTime, crntTime);
	renderer.getTiler().updateTiles(*mainCam);
	events.updateAllEvents(prevUpdateTime, crntTime);

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
		updateSceneNode(*n, prevUpdateTime, crntTime, sectorGroup);
	}
#else
	Array<UpdateSceneNodesJob, ThreadPool::MAX_THREADS> jobs2;

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

	threadPool.waitForAllJobsToFinish();
#endif

	doVisibilityTests(*mainCam, *this, renderer);

	/*sectorGroup.doVisibilityTests(*mainCam,
		VisibilityTest(VT_RENDERABLES | VT_LIGHTS), &r);*/

	ANKI_COUNTER_STOP_TIMER_INC(C_SCENE_UPDATE_TIME);
}

//==============================================================================
SceneNode& SceneGraph::findSceneNode(const char* name)
{
	ANKI_ASSERT(nameToNode.find(name) != nameToNode.end());
	return *(nameToNode.find(name)->second);
}

//==============================================================================
SceneNode* SceneGraph::tryFindSceneNode(const char* name)
{
	Types<SceneNode>::NameToItemMap::iterator it = nameToNode.find(name);
	return (it == nameToNode.end()) ? nullptr : it->second;
}

//==============================================================================
void SceneGraph::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(ANKI_R(filename));

		XmlElement rootEl = doc.getChildElement("scene");

		// Model nodes
		//
		XmlElement mdlNodeEl = rootEl.getChildElement("modelNode");

		do
		{
			XmlElement el, el1;
	
			// <name>
			el = mdlNodeEl.getChildElement("name");
			std::string name = el.getText();

			// <model>
			el = mdlNodeEl.getChildElement("model");

			// <instancesCount>
			el1 = mdlNodeEl.getChildElementOptional("instancesCount");
			U32 instancesCount = (el1) ? el1.getInt() : 1;

			if(instancesCount > ANKI_MAX_INSTANCES)
			{
				throw ANKI_EXCEPTION("Too many instances");
			}

			ModelNode* node = ANKI_NEW(ModelNode, alloc,
				name.c_str(), this, nullptr,
				Movable::MF_NONE, el.getText(), instancesCount);

			// <transform>
			el = mdlNodeEl.getChildElement("transform");
			U i = 0;

			do
			{
				if(i == 0)
				{
					node->setLocalTransform(Transform(el.getMat4()));
					node->setInstanceLocalTransform(
						i, Transform(el.getMat4()));
				}
				else
				{
					node->setInstanceLocalTransform(i, Transform(el.getMat4()));
				}

				// Advance
				el = el.getNextSiblingElement("transform");
				++i;
			}
			while(el && i < instancesCount);

			if(i != instancesCount)
			{
				throw ANKI_EXCEPTION("instancesCount does not match "
					"with transform");
			}

			// Advance
			mdlNodeEl = mdlNodeEl.getNextSiblingElement("modelNode");
		} while(mdlNodeEl);
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Sceno loading failed") << e;
	}
}

} // end namespace anki
