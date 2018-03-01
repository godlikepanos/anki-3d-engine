// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SceneGraph.h>
#include <anki/scene/CameraNode.h>
#include <anki/scene/ModelNode.h>
#include <anki/scene/SectorNode.h>
#include <anki/core/Trace.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/resource/ResourceManager.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/ThreadPool.h>

namespace anki
{

const U NODE_UPDATE_BATCH = 10;

class UpdateSceneNodesCtx
{
public:
	SceneGraph* m_scene = nullptr;

	IntrusiveList<SceneNode>::Iterator m_crntNode;
	SpinLock m_crntNodeLock;

	Second m_prevUpdateTime;
	Second m_crntTime;
};

class UpdateSceneNodesTask : public ThreadPoolTask
{
public:
	UpdateSceneNodesCtx* m_ctx;

	Error operator()(U32 taskId, PtrSize threadsCount)
	{
		return m_ctx->m_scene->updateNodes(*m_ctx);
	}
};

SceneGraph::SceneGraph()
{
}

SceneGraph::~SceneGraph()
{
	Error err = iterateSceneNodes([&](SceneNode& s) -> Error {
		s.setMarkedForDeletion();
		return Error::NONE;
	});
	(void)err;

	deleteNodesMarkedForDeletion();

	if(m_sectors)
	{
		m_alloc.deleteInstance(m_sectors);
		m_sectors = nullptr;
	}
}

Error SceneGraph::init(AllocAlignedCallback allocCb,
	void* allocCbData,
	ThreadPool* threadpool,
	ThreadHive* threadHive,
	ResourceManager* resources,
	Input* input,
	ScriptManager* scriptManager,
	const Timestamp* globalTimestamp,
	const ConfigSet& config)
{
	m_globalTimestamp = globalTimestamp;
	m_threadpool = threadpool;
	m_threadHive = threadHive;
	m_resources = resources;
	m_objectsMarkedForDeletionCount.store(0);
	m_gr = &m_resources->getGrManager();
	m_physics = &m_resources->getPhysicsWorld();
	m_input = input;
	m_scriptManager = scriptManager;

	m_alloc = SceneAllocator<U8>(allocCb, allocCbData, 1024 * 10, 1.0, 0);
	m_frameAlloc = SceneFrameAllocator<U8>(allocCb, allocCbData, 1 * 1024 * 1024);

	m_earlyZDist = config.getNumber("scene.earlyZDistance");

	ANKI_CHECK(m_events.create(this));

	m_sectors = m_alloc.newInstance<SectorGroup>(this);

	m_maxReflectionProxyDistance = config.getNumber("scene.imageReflectionMaxDistance");

	// Init the default main camera
	ANKI_CHECK(newSceneNode<PerspectiveCameraNode>("mainCamera", m_defaultMainCam));
	m_defaultMainCam->setAll(toRad(60.0f), toRad(60.0f), 0.1f, 1000.0f);
	m_mainCam = m_defaultMainCam;

	return Error::NONE;
}

Error SceneGraph::registerNode(SceneNode* node)
{
	ANKI_ASSERT(node);

	// Add to dict if it has a name
	if(node->getName())
	{
		if(tryFindSceneNode(node->getName()))
		{
			ANKI_SCENE_LOGE("Node with the same name already exists");
			return Error::USER_DATA;
		}

		m_nodesDict.emplace(m_alloc, node->getName(), node);
	}

	// Add to vector
	m_nodes.pushBack(node);
	++m_nodesCount;

	return Error::NONE;
}

void SceneGraph::unregisterNode(SceneNode* node)
{
	// Remove from the graph
	m_nodes.erase(node);
	--m_nodesCount;

	if(m_mainCam != m_defaultMainCam && m_mainCam == node)
	{
		m_mainCam = m_defaultMainCam;
	}

	// Remove from dict
	if(node->getName())
	{
		auto it = m_nodesDict.find(node->getName());
		ANKI_ASSERT(it != m_nodesDict.getEnd());
		m_nodesDict.erase(m_alloc, it);
	}
}

SceneNode& SceneGraph::findSceneNode(const CString& name)
{
	SceneNode* node = tryFindSceneNode(name);
	ANKI_ASSERT(node);
	return *node;
}

SceneNode* SceneGraph::tryFindSceneNode(const CString& name)
{
	auto it = m_nodesDict.find(name);
	return (it == m_nodesDict.getEnd()) ? nullptr : (*it);
}

void SceneGraph::deleteNodesMarkedForDeletion()
{
	/// Delete all nodes pending deletion. At this point all scene threads
	/// should have finished their tasks
	while(m_objectsMarkedForDeletionCount.load() > 0)
	{
		Bool found = false;
		auto it = m_nodes.begin();
		auto end = m_nodes.end();
		for(; it != end; ++it)
		{
			SceneNode& node = *it;

			if(node.getMarkedForDeletion())
			{
				// Delete node
				unregisterNode(&node);
				m_alloc.deleteInstance(&node);
				m_objectsMarkedForDeletionCount.fetchSub(1);
				found = true;
				break;
			}
		}

		(void)found;
		ANKI_ASSERT(found && "Something is wrong with marked for deletion");
	}
}

Error SceneGraph::update(Second prevUpdateTime, Second crntTime)
{
	ANKI_ASSERT(m_mainCam);
	ANKI_TRACE_SCOPED_EVENT(SCENE_UPDATE);

	m_stats.m_updateTime = HighRezTimer::getCurrentTime();

	m_timestamp = *m_globalTimestamp;

	// Reset the framepool
	m_frameAlloc.getMemoryPool().reset();

	// Delete stuff
	ANKI_TRACE_START_EVENT(SCENE_DELETE_STUFF);
	m_events.deleteEventsMarkedForDeletion();
	deleteNodesMarkedForDeletion();
	ANKI_TRACE_STOP_EVENT(SCENE_DELETE_STUFF);

	ThreadPool& threadPool = *m_threadpool;
	(void)threadPool;

	// Update
	ANKI_TRACE_START_EVENT(SCENE_PHYSICS_UPDATE);
	m_physics->updateAsync(crntTime - prevUpdateTime);
	m_physics->waitUpdate();
	ANKI_TRACE_STOP_EVENT(SCENE_PHYSICS_UPDATE);

	ANKI_TRACE_START_EVENT(SCENE_NODES_UPDATE);
	ANKI_CHECK(m_events.updateAllEvents(prevUpdateTime, crntTime));

	// Then the rest
	Array<UpdateSceneNodesTask, ThreadPool::MAX_THREADS> jobs2;
	UpdateSceneNodesCtx updateCtx;
	updateCtx.m_scene = this;
	updateCtx.m_crntNode = m_nodes.getBegin();
	updateCtx.m_prevUpdateTime = prevUpdateTime;
	updateCtx.m_crntTime = crntTime;

	for(U i = 0; i < threadPool.getThreadCount(); i++)
	{
		UpdateSceneNodesTask& job = jobs2[i];
		job.m_ctx = &updateCtx;
		threadPool.assignNewTask(i, &job);
	}

	ANKI_CHECK(threadPool.waitForAllThreadsToFinish());
	ANKI_TRACE_STOP_EVENT(SCENE_NODES_UPDATE);

	m_stats.m_updateTime = HighRezTimer::getCurrentTime() - m_stats.m_updateTime;
	return Error::NONE;
}

void SceneGraph::doVisibilityTests(RenderQueue& rqueue)
{
	m_stats.m_visibilityTestsTime = HighRezTimer::getCurrentTime();
	anki::doVisibilityTests(*m_mainCam, *this, rqueue);
	m_stats.m_visibilityTestsTime = HighRezTimer::getCurrentTime() - m_stats.m_visibilityTestsTime;
}

Error SceneGraph::updateNode(Second prevTime, Second crntTime, SceneNode& node)
{
	ANKI_TRACE_INC_COUNTER(SCENE_NODES_UPDATED, 1);

	Error err = Error::NONE;

	// Components update
	Timestamp componentTimestam = 0;
	err = node.iterateComponents([&](SceneComponent& comp) -> Error {
		Bool updated = false;
		Error e = comp.updateReal(node, prevTime, crntTime, updated);
		componentTimestam = max(componentTimestam, comp.getTimestamp());

		return e;
	});

	// Update children
	if(!err)
	{
		err = node.visitChildren([&](SceneNode& child) -> Error { return updateNode(prevTime, crntTime, child); });
	}

	// Frame update
	if(!err)
	{
		err = node.frameUpdateComplete(prevTime, crntTime, componentTimestam);
	}

	return err;
}

Error SceneGraph::updateNodes(UpdateSceneNodesCtx& ctx) const
{
	ANKI_TRACE_START_EVENT(SCENE_NODES_UPDATE);

	IntrusiveList<SceneNode>::Iterator& it = ctx.m_crntNode;
	IntrusiveList<SceneNode>::ConstIterator end = m_nodes.getEnd();
	SpinLock& lock = ctx.m_crntNodeLock;

	Bool quit = false;
	Error err = Error::NONE;
	while(!quit && !err)
	{
		// Fetch a few scene nodes
		Array<SceneNode*, NODE_UPDATE_BATCH> nodes = {{
			nullptr,
		}};
		lock.lock();
		for(SceneNode*& node : nodes)
		{
			if(it != end)
			{
				node = &(*it);
				++it;
			}
		}
		lock.unlock();

		// Process nodes
		U count = 0;
		for(U i = 0; i < nodes.getSize(); ++i)
		{
			if(nodes[i])
			{
				if(nodes[i]->getParent() == nullptr)
				{
					err = updateNode(ctx.m_prevUpdateTime, ctx.m_crntTime, *nodes[i]);
				}
				++count;
			}
		}

		if(ANKI_UNLIKELY(count == 0))
		{
			quit = true;
		}
	}

	ANKI_TRACE_STOP_EVENT(SCENE_NODES_UPDATE);
	return err;
}

} // end namespace anki
