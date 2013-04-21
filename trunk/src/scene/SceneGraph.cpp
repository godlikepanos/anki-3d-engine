#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/util/Exception.h"
#include "anki/core/ThreadPool.h"
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
	sn.frameUpdate(prevUpdateTime, crntTime, Timestamp::getTimestamp());

	// Do some spatial stuff
	Spatial* sp = sn.getSpatial();
	if(sp)
	{
		if(sp->getSpatialTimestamp() >= Timestamp::getTimestamp())
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
	:	alloc(ANKI_CFG_SCENE_ALLOCATOR_SIZE),
		frameAlloc(ANKI_CFG_SCENE_FRAME_ALLOCATOR_SIZE),
		nodes(alloc),
		sectorGroup(this),
		events(this)
{
	nodes.reserve(ANKI_CFG_SCENE_NODES_AVERAGE_COUNT);

	ambientCol = Vec3(0.1, 0.05, 0.05) * 3;
}

//==============================================================================
SceneGraph::~SceneGraph()
{}

//==============================================================================
void SceneGraph::registerNode(SceneNode* node)
{
	addC(nodes, node);
	addDict(nameToNode, node);
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	removeC(nodes, node);
	removeDict(nameToNode, node);
}

//==============================================================================
void SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
#if ANKI_CFG_SCENE_PROFILE
	HighRezTimer::Scalar startTime = HighRezTimer::getCurrentTime();
#endif

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

#if ANKI_CFG_SCENE_PROFILE
	timeForUpdates += HighRezTimer::getCurrentTime() - startTime;
#endif
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
void SceneGraph::printProfileInfo() const
{
#if ANKI_CFG_SCENE_PROFILE
	ANKI_LOGI("Scene times: " << timeForUpdates);
#endif
}

//==============================================================================
void SceneGraph::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);

		XmlElement rootEl = doc.getChildElement("scene");

		// Model nodes
		//
		XmlElement mdlNodeEl = rootEl.getChildElement("modelNode");

		do
		{
			XmlElement el;
	
			// <model>
			el = mdlNodeEl.getChildElement("model");

			ModelNode* node = new ModelNode(el.getText(), "name", this, 
				Movable::MF_NONE, nullptr);

			// <transform>
			el = mdlNodeEl.getChildElement("transform");
			StringList list = StringList::splitString(el.getText(), ' ');

			if(list.size() != 16)
			{
				throw ANKI_EXCEPTION("Expecting 16 floats for <transform>");
			}

			Mat4 trf;
			for(U i = 0; i < 16; i++)
			{
				trf[i] = std::stof(list[i]);
			}

			node->setLocalTransform(Transform(trf));

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
