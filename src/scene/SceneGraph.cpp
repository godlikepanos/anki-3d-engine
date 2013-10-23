#include "anki/scene/SceneGraph.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/util/Exception.h"
#include "anki/core/Threadpool.h"
#include "anki/core/Counters.h"
#include "anki/renderer/Renderer.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct UpdateMoveComponentsJob: ThreadpoolTask
{
	SceneGraph* scene = nullptr;

	void operator()(ThreadId threadId, U threadsCount)
	{
		U64 start, end;
		ANKI_ASSERT(scene);
		choseStartEnd(
			threadId, threadsCount, scene->getSceneNodesCount(), start, end);

		scene->iterateSceneNodes(start, end, [](SceneNode& sn)
		{
			MoveComponent* m = sn.getMoveComponent();
			if(m)
			{
				m->update();
			}
		});
	}
};

//==============================================================================
static void updateSceneNode(SceneNode& sn, F32 prevUpdateTime,
	F32 crntTime, SectorGroup& sectorGroup)
{
	sn.frameUpdate(prevUpdateTime, crntTime, getGlobTimestamp());

	// Do some spatial stuff
	SpatialComponent* sp = sn.getSpatialComponent();
	if(sp)
	{
		sp->resetFrame();
	}

	// Do some frustumable stuff
	FrustumComponent* fr = sn.getFrustumComponent();
	if(fr)
	{
		fr->resetFrame();
	}

	// Do some renderable stuff
	RenderComponent* r = sn.getRenderComponent();
	if(r)
	{
		r->resetFrame();
	}
}

//==============================================================================
struct UpdateSceneNodesJob: ThreadpoolTask
{
	SceneGraph* scene = nullptr;
	F32 prevUpdateTime;
	F32 crntTime;
	SectorGroup* sectorGroup;

	void operator()(ThreadId threadId, U threadsCount)
	{
		ANKI_ASSERT(scene);
		U64 start, end;
		choseStartEnd(
			threadId, threadsCount, scene->getSceneNodesCount(), start, end);

		scene->iterateSceneNodes(start, end, [&](SceneNode& sn)
		{
			updateSceneNode(sn, prevUpdateTime, crntTime, *sectorGroup);	
		});
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
		dict(10, DictionaryHasher(), DictionaryEqual(), alloc),
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
	ANKI_ASSERT(node);

	// Add to dict if it has name
	if(node->getName())
	{
		if(dict.find(node->getName()) != dict.end())
		{
			throw ANKI_EXCEPTION("Node with the same name already exists: "
				+ node->getName());
		}

		dict[node->getName()] = node;
	}

	// Add to vector
	ANKI_ASSERT(std::find(nodes.begin(), nodes.end(), node) == nodes.end());
	nodes.push_back(node);
}

//==============================================================================
void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from vector
	auto it = nodes.begin();
	for(; it != nodes.end(); it++)
	{
		if((*it) == node)
		{
			break;
		}
	}
	
	ANKI_ASSERT(it != nodes.end());
	nodes.erase(it);

	// Remove from dict
	if(node->getName())
	{
		auto it = dict.find(node->getName());

		ANKI_ASSERT(it != dict.end());
		dict.erase(it);
	}
}

//==============================================================================
SceneNode& SceneGraph::findSceneNode(const char* name)
{
	ANKI_ASSERT(dict.find(name) != dict.end());
	return *(dict.find(name))->second;
}

//==============================================================================
SceneNode* SceneGraph::tryFindSceneNode(const char* name)
{
	auto it = dict.find(name);
	return (it == dict.end()) ? nullptr : it->second;
}

//==============================================================================
void SceneGraph::deleteNodesMarkedForDeletion()
{
	/// Delete all nodes pending deletion. At this point all scene threads 
	/// should have finished their tasks
	while(nodesMarkedForDeletionCount > 0)
	{
		// First gather the nodes that will be de
		SceneFrameVector<decltype(nodes)::iterator> forDeletion;
		for(auto it = nodes.begin(); it != nodes.end(); it++)
		{
			if((*it)->isMarkedForDeletion())
			{
				forDeletion.push_back(it);
			}
		}

		// Now delete
		for(auto& it : forDeletion)
		{
			// Disable events for that node
			events.iterateEvents([&](Event& e)
			{
				if(e.getSceneNode() == *it)
				{
					e.markForDeletion();
				}
			});

			// Remove it
			unregisterNode(*it);

			SceneAllocator<SceneNode> al = alloc;
			alloc.destroy(*it);
			alloc.deallocate(*it, 1);

			++nodesMarkedForDeletionCount;
		}
	}
}

//==============================================================================
void SceneGraph::update(F32 prevUpdateTime, F32 crntTime, Renderer& renderer)
{
	ANKI_ASSERT(mainCam);

	ANKI_COUNTER_START_TIMER(C_SCENE_UPDATE_TIME);

	//
	// Sync point. Here we wait for all scene's threads
	//

	// XXX

	//
	// Reset the frame mem pool
	//
	frameAlloc.reset();

	deleteNodesMarkedForDeletion();

	Threadpool& threadPool = ThreadpoolSingleton::get();
	(void)threadPool;

	// XXX Do that in parallel
	physics.update(prevUpdateTime, crntTime);
	renderer.getTiler().updateTiles(*mainCam);
	events.updateAllEvents(prevUpdateTime, crntTime);

#if 0
	// First do the movable updates
	for(SceneNode* n : nodes)
	{
		MoveComponent* m = n->getMoveComponent();

		if(m)
		{
			m->update();
		}
	}
#else
	UpdateMoveComponentsJob jobs[Threadpool::MAX_THREADS];

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		jobs[i].scene = this;

		threadPool.assignNewTask(i, &jobs[i]);
	}

	threadPool.waitForAllThreadsToFinish();
#endif

	// Then the rest
#if 0
	for(SceneNode* n : nodes)
	{
		updateSceneNode(*n, prevUpdateTime, crntTime, sectorGroup);
	}
#else
	Array<UpdateSceneNodesJob, Threadpool::MAX_THREADS> jobs2;

	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		UpdateSceneNodesJob& job = jobs2[i];

		job.scene = this;
		job.prevUpdateTime = prevUpdateTime;
		job.crntTime = crntTime;
		job.sectorGroup = &sectorGroup;

		threadPool.assignNewTask(i, &job);
	}

	threadPool.waitForAllThreadsToFinish();
#endif

	doVisibilityTests(*mainCam, *this, renderer);

	/*sectorGroup.doVisibilityTests(*mainCam,
		VisibilityTest(VT_RENDERABLES | VT_LIGHTS), &r);*/

	ANKI_COUNTER_STOP_TIMER_INC(C_SCENE_UPDATE_TIME);
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

			ModelNode* node;
			newSceneNode(node, name.c_str(), el.getText(), instancesCount);

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
		throw ANKI_EXCEPTION("Scene loading failed") << e;
	}
}

} // end namespace anki
